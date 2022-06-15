#pragma once
#include "pixel_api.h"
#include "tile.h"

namespace alc {

void update_sand(tile::pixels& pixels, glm::ivec2 pos, double dt);
void update_water(tile::pixels& pixels, glm::ivec2 pos, double dt);
void update_rock(tile::pixels& pixels, glm::ivec2 pos, double dt);
    
}