#pragma once
#include <glm/glm.hpp>

namespace alc {

// Static Settings
static constexpr float TERMINAL_VELOCITY = 5.0f;
static constexpr int PIXELS_PER_METER = 10;

// Runtime Settings
struct world_settings
{
    glm::vec2 gravity;
};

}