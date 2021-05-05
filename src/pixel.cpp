#include "pixel.h"

namespace alc {

pixel pixel::air()
{
    return {
        pixel_type::air,
        { 44.0f / 256.0f, 58.0f / 256.0f, 71.0f / 256.0f, 1.0 },
        false
    };
}

pixel pixel::sand()
{
    return {
        pixel_type::sand,
        { 248.0f / 256.0f, 239.0f / 256.0f, 186.0f / 256.0f, 1.0 },
        false
    };
}

}