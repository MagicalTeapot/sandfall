#pragma once
#include "tile.h"

#include <glm/glm.hpp>

namespace alc {

class pixel_api
// A light wrapper around pixel data to make the material update functions simpler.
{
    tile::pixels&         d_pixels_ref;
    glm::ivec2            d_pos;

public:
    pixel_api(tile::pixels& pixels_ref, glm::ivec2 pos)
        : d_pixels_ref(pixels_ref), d_pos(pos) {}

    glm::ivec2 move_to(glm::ivec2 offset);
    pixel& get(glm::ivec2 offset);
};

}