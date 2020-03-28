
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <stdexcept>
#include "colib/co_mutex.h"

#include "Logic.hpp"
#include "../define.hpp"


int DoCheckDisk(const char *device)
{
    printf("Checking %s\n", device);
    int fd = open(device, O_RDWR);
    if (fd == -1)
    {
        puts("open() failed");
        return fd;
    }
    auto disk_size = lseek(fd, 0, SEEK_END);
    if (disk_size == -1)
    {
        puts("lseek() failed");
        return disk_size;
    }
    constexpr auto chunk_size = 1LU << 31; // 2 GiB
    auto chunk_count = disk_size / chunk_size;
    for (decltype(chunk_count) chunk_i = 0; chunk_i < chunk_count; ++chunk_i)
    {
        auto base_offset = chunk_i * chunk_size;
        printf("Chunk #%04lu at offset 0x%016lx ", chunk_i, base_offset);
        ChunkHeader ch;
        pread64(fd, &ch, sizeof(ch), base_offset + 0);
        if (strcmp(ch.Magic, "ALOHA") != 0)
        {
            puts("magic number not correct");
            continue;
        }
        printf("version: %d, hex: 0x%016lx, next_inode: %u, l_used: %u, a_used: %u(4k aligned)\n", 
            ch.Version, ch.ChunkId, ch.NextInode, ch.LogicalUsedSpace, ch.ActualUsedSpace);
    }
    close(fd);
    return chunk_count;
}


void DoInitialize(const char *conf_file)
{
    // Parse config file
    puts("Parsing business library config file...");
    int ret = LibConf.ParseFile(conf_file);
    if (ret != 0)
    {
        throw std::runtime_error(string("Parse config file failed: ").append(conf_file));
    }
    GClusterId = atoi(LibConf.GetKV("root", "cluster_id").c_str());
    GMachineId = atoi(LibConf.GetKV("root", "machine_id").c_str());
    printf("%lu %lu\n", GClusterId, GMachineId);
    GDiskCount = LibConf.GetSection("root\\disks").Children.size();
    GDiskInfo = new DiskInfo[GDiskCount];
    for (const auto &i : LibConf.GetSection("root\\disks").Children)
    {
        const auto &tag = i.Tag;
        const auto &device = LibConf.GetKV(string("root\\disks\\").append(tag).c_str(), "path");
        int disk_id = atoi(LibConf.GetKV(string("root\\disks\\").append(tag).c_str(), "disk_id").c_str());
        auto &di = GDiskInfo[disk_id];
        di.ChunkCount = DoCheckDisk(device.c_str());
        if (di.ChunkCount < 0)
        {
            printf("check failed, retcode = %d\n", di.ChunkCount);
        }
        di.Fd = open(device.c_str(), O_RDWR | O_DIRECT);
    }
}