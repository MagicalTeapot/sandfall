#pragma once
#include <glm/glm.hpp>

namespace alc {

enum class pixel_type
{
    air,
    sand,
    rock,
    water,
    red_sand
};

struct pixel
{
    // Static Data
    pixel_type type;
    glm::vec4  colour;

    // Dynamic Data
    bool updated_this_frame = false;

    static pixel air();
    static pixel sand();
    static pixel rock();
    static pixel water();
    static pixel red_sand();
};

}