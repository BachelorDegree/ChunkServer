#pragma once

#include <cstdint>
#include "DiskInfo.hpp"
#include "../CoreDeps/include/TfcConfigCodec.hpp"

extern AlohaIO::TfcConfigCodec LibConf;
extern uint64_t GClusterId, GMachineId;
extern uint32_t GDiskCount;
extern DiskInfo* GDiskInfo;

void DoInitialize(const char *conf_file);
