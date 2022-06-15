#include "update_functions.h"

#include <array>
#include <utility>

#include <glm/glm.hpp>

namespace alc {
namespace {

std::size_t get_pos(glm::ivec2 pos)
{
    return pos.x + alc::tile::SIZE * pos.y;
}

}

void update_sand(tile::pixels& pixels, glm::ivec2 pos, double dt)
{
    auto x_offsets = std::array{0, -1, 1};
    if (rand() % 2) {
        std::swap(x_offsets[1], x_offsets[2]);
    }

    for (auto dx : x_offsets) {
        const auto new_pos = pos + glm::ivec2{dx, 1};
        if (tile::valid(new_pos) && pixels[get_pos(new_pos)].type == pixel_type::air) {
            std::swap(pixels[get_pos(pos)], pixels[get_pos(new_pos)]);
            return;
        }
    }
}

void update_water(tile::pixels& pixels, glm::ivec2 pos, double dt)
{
    //auto offset = glm::ivec2{0, 1};
    //
    //if (api.move_to(offset) != glm::ivec2{0, 0}) {
    //    return;
    //}
//
    //std::array<glm::ivec2, 4> offsets = {
    //    glm::ivec2{-1, 1}, glm::ivec2{1, 1}, glm::ivec2{-1, 0}, glm::ivec2{1, 0}
    //};
//
    //if (rand() % 2) {
    //    std::swap(offsets[0], offsets[1]);
    //    std::swap(offsets[2], offsets[3]);
    //}
//
    //for (auto offset : offsets) {
    //    if (api.move_to(offset) != glm::ivec2{0, 0}) {
    //        return;
    //    }
    //}
}

void update_rock(tile::pixels& pixels, glm::ivec2 pos, double dt)
{
    pixels[get_pos(pos)].updated_this_frame = true;
}

}