#include <audio/AL/alutil.h>

#include <fstream>
#include <cstring>
#include <memory>
#include <type_traits>

#ifdef __APPLE__
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#else
#include <AL/al.h>
#include <AL/alc.h>
#endif

#include <debug/Logger.h>

static debug::Logger logger("alutil");

bool AL::check_errors(const std::string& filename, const std::uint_fast32_t line){
    ALenum error = alGetError();
    if(error != AL_NO_ERROR){
        logger.error() << "OpenAL ERROR (" << filename << ": " << line << ")";
        switch(error){
        case AL_INVALID_NAME:
            logger.error() << "AL_INVALID_NAME: a bad name (ID) was passed to an OpenAL function";
            break;
        case AL_INVALID_ENUM:
            logger.error() << "AL_INVALID_ENUM: an invalid enum value was passed to an OpenAL function";
            break;
        case AL_INVALID_VALUE:
            logger.error() << "AL_INVALID_VALUE: an invalid value was passed to an OpenAL function";
            break;
        case AL_INVALID_OPERATION:
            logger.error() << "AL_INVALID_OPERATION: the requested operation is not valid";
            break;
        case AL_OUT_OF_MEMORY:
            logger.error() << "AL_OUT_OF_MEMORY: the requested operation resulted in OpenAL running out of memory";
            break;
        default:
            logger.error() << "UNKNOWN AL ERROR: " << (int)error;
        }
        return false;
    }
    return true;
}
