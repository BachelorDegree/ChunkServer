#include <thread>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdexcept>
#include <spdlog/spdlog.h>
#include <colib/co_mutex.h>

#include "Logic.hpp"
#include "InodeLruCache.hpp"
#include "ReportToMasterLoop.hpp"

#include "coredeps/SatelliteClient.hpp"


static int DoCheckDisk(const char *device, DiskInfo &di)
{
    spdlog::info("Checking disk {}", device);
    int fd = open(device, O_RDWR);
    if (fd == -1)
    {
        spdlog::error("open() failed");
        return fd;
    }
    auto disk_size = lseek(fd, 0, SEEK_END);
    if (disk_size == -1)
    {
        spdlog::error("lseek() failed");
        return disk_size;
    }
    constexpr auto chunk_size = 1LU << 31; // 2 GiB
    auto chunk_count = disk_size / chunk_size;
    di.ChunkCount = chunk_count;
    di.Chunks = new ChunkInfo[di.ChunkCount];
    for (decltype(chunk_count) chunk_i = 0; chunk_i < chunk_count; ++chunk_i)
    {
        auto base_offset = chunk_i * chunk_size;
        spdlog::info("Chunk #{:04d} at offset 0x{:016x}", chunk_i, base_offset);
        ChunkHeader ch;
        pread64(fd, &ch, sizeof(ch), base_offset + 0);
        if (strcmp(ch.Magic, "ALOHA") != 0)
        {
            spdlog::error("magic number not correct!");
            continue;
        }
        spdlog::info("hex: 0x{:016x}, next_inode: {}, l_used: {}, a_used: {}, state: {}",
            ch.ChunkId, ch.NextInode, ch.LogicalUsedSpace, ch.ActualUsedSpace, ch.State);
        auto &ci = di.Chunks[chunk_i];
        ci.SetDiskInfoPtr(&di);
        ci.ChunkId = ch.ChunkId;
        ci.NextInode = ch.NextInode;
        ci.ActualUsedSpace = ch.ActualUsedSpace;
        ci.LogicalUsedSpace = ch.LogicalUsedSpace;
        ci.State = ch.State;
    }
    close(fd);
    return chunk_count;
}

static void DoInitializeInodeCache(void)
{
    auto size = atoi(LibConf.GetKV("root\\inode_cache", "mempool_size").c_str());
    spdlog::info("Initialize inode LRU cache, mempool size: {}", size);
    InodeLruCache::GetInstance().InitializeMemoryPool(size);
}

void DoInitialize(const char *conf_file)
{
    // Parse config file
    spdlog::info("Parsing business library config file...");
    int ret = LibConf.ParseFile(conf_file);
    if (ret != 0)
    {
        throw std::runtime_error(string("Parse config file failed: ").append(conf_file));
    }
    g_iClusterId = atoi(LibConf.GetKV("root", "cluster_id").c_str());
    g_iMachineId = atoi(LibConf.GetKV("root", "machine_id").c_str());
    spdlog::info("Cluster ID: {}, Machine ID: {}", g_iClusterId, g_iMachineId);
    g_iDiskCount = LibConf.GetSection("root\\disks").Children.size();
    g_apDiskInfo = new DiskInfo[g_iDiskCount];
    for (const auto &i : LibConf.GetSection("root\\disks").Children)
    {
        const auto &tag = i.Tag;
        const auto &device = LibConf.GetKV(string("root\\disks\\").append(tag).c_str(), "path");
        int disk_id = atoi(LibConf.GetKV(string("root\\disks\\").append(tag).c_str(), "disk_id").c_str());
        auto &di = g_apDiskInfo[disk_id];
        DoCheckDisk(device.c_str(), di);
        if (di.ChunkCount < 0)
        {
            spdlog::error("check failed, retcode = {}", di.ChunkCount);
        }
        di.Fd = open(device.c_str(), O_RDWR);
        spdlog::info("Opening {}, fd={}", tag, di.Fd);
    }
    // Inode LRU cache
    DoInitializeInodeCache();
    // Start report to master loop
    std::thread thReportLoop(ReportToMasterLoop);
    thReportLoop.detach();
}