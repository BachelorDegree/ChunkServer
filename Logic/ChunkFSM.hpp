#pragma once

class ChunkFSM
{
public:
    enum Status
    {
        STANDBY     = 0,
        WRITING     = 1,
        ERROR       = 2,
        MOVING      = 3,
        EMPTY       = 4,
        EMPTYERROR  = 5,
        NOTKNOWN    = 6
    };

    enum Action
    {
        SET_DIRTY,
        RECOVERED,
        ALLOCATE_UPLOAD,
        FINISH_UPLOAD,
        START_MOVING,
        FINISH_MOVING_AS_SOURCE,
        FINISH_MOVING_AS_DESTINATION
    };

    static Status Transit(Status iCurrentState, Action iAction);
    inline static uint32_t Transit(uint32_t iCurrentState, Action iAction)
    {
        return static_cast<uint32_t>(ChunkFSM::Transit(static_cast<Status>(iCurrentState), iAction));
    }
};

inline bool operator == (const ChunkFSM::Status& lhs, const uint32_t& rhs)
{
    return static_cast<uint32_t>(lhs) == rhs;
}

inline bool operator == (const uint32_t& lhs, const ChunkFSM::Status& rhs)
{
    return lhs == static_cast<uint32_t>(rhs);
}
