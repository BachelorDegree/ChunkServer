#pragma once

enum CHUNKSERVER_ERROR_CODE // Range -2000 ~ -2200
{
    // AllocatedInode, 2010~2019
    E_DISK_NOT_ON_THIS_MACHINE = -2010,
    E_CHUNK_ID_OUT_OF_RANGE = -2011,
    E_FLUSH_INODE_FAILED = -2012,
    E_FLUSH_CHUNK_HEADER_FAILED = -2013,
    E_SLICE_NUMBER_DOES_NOT_MATCH_CHUNK_STATUS = -2014,
    // WriteSliceHandler, 2020~2029
    E_OFFSET_OUT_OF_RANGE = -2020,
    E_DATA_LENGTH_OUT_OF_RANGE = -2021,
    E_PREAD_FAILED = -2022,
    E_PWRITE_FAILED = -2023,
    // ManipulateReferenceCount, 2030~2039
    E_MANIP_REFCOUNT_OP_UNKNOWN = -2030,
    E_REFCOUNT_IS_ZERO = -2031,
};
