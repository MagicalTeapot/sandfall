#pragma once
#include "pixel_api.h"

namespace alc {

void update_sand(pixel_api&& api, double dt);
void update_water(pixel_api&& api, double dt);
void update_rock(pixel_api&& api, double dt);
    
}