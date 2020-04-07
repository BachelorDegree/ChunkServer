#pragma once

#include <cstdint>
#include "DiskInfo.hpp"
#include "coredeps/TfcConfigCodec.hpp"

extern AlohaIO::TfcConfigCodec LibConf;
extern uint64_t g_iClusterId, g_iMachineId;
extern uint32_t g_iDiskCount;
extern DiskInfo* g_apDiskInfo;

void DoInitialize(const char *);
int  IsChunkIdValid(uint64_t iChunkId);
