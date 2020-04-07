#include "ChunkFSM.hpp"

ChunkFSM::Status ChunkFSM::Transit(ChunkFSM::Status iCurrentState, ChunkFSM::Action iAction)
{
    switch (iCurrentState)
    {
    case STANDBY:
        switch (iAction)
        {
        case ALLOCATE_UPLOAD:
            return WRITING;
        case START_MOVING:
            return MOVING;
        default:
            return NOTKNOWN;
        }
    case WRITING:
        switch (iAction)
        {
        case SET_DIRTY:
            return ERROR;
        case FINISH_UPLOAD:
            return STANDBY;
        default:
            return NOTKNOWN;
        }
    case ERROR:
        switch (iAction)
        {
        case SET_DIRTY:
            return ERROR;
        case RECOVERED:
            return STANDBY;
        case FINISH_MOVING_AS_SOURCE:
            return EMPTYERROR;
        default:
            return NOTKNOWN;
        }
    case MOVING:
        switch (iAction)
        {
        case FINISH_MOVING_AS_SOURCE:
            return EMPTY;
        default:
            return NOTKNOWN;
        }
    case EMPTY:
        switch (iAction)
        {
        case FINISH_MOVING_AS_DESTINATION:
            return STANDBY;
        case SET_DIRTY:
            return EMPTYERROR;
        default:
            return NOTKNOWN;
        }
    case EMPTYERROR:
        switch (iAction)
        {
        case SET_DIRTY:
            return EMPTYERROR;
        case RECOVERED:
            return EMPTY;
        default:
            return NOTKNOWN;
        }
    case NOTKNOWN:
    default:
        return NOTKNOWN;
    }
}
