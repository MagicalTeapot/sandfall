#include "tile.h"
#include "update_functions.h"
#include "utility/log.h"

#include <glad/glad.h>

#include <cassert>
#include <algorithm>

namespace sand {
namespace {

auto get_pos(glm::vec2 pos) -> std::size_t
{
    return pos.x + tile_size * pos.y;
}

}

tile::tile()
{
    const auto default_pixel = pixel::air();
    d_pixels.fill(default_pixel);
    d_buffer.fill(default_pixel.colour);
}

auto tile::valid(glm::ivec2 pos) -> bool
{
    return 0 <= pos.x && pos.x < tile_size && 0 <= pos.y && pos.y < tile_size;
}

auto tile::simulate(const world_settings& settings, double dt) -> void
{
    const auto inner = [&] (std::uint32_t x, std::uint32_t y) {
        auto& pixel = d_pixels[get_pos({x, y})];
        if (pixel.updated_this_frame) { return; }

        // TODO: Use std::visit
        if (std::holds_alternative<movable_solid>(pixel.data)) {
            update_sand(*this, {x, y}, settings, dt);
        }
        else if (std::holds_alternative<static_solid>(pixel.data)) {
            update_rock(*this, {x, y}, settings, dt);
        }
        else if (std::holds_alternative<liquid>(pixel.data)) {
            update_water(*this, {x, y}, settings, dt);
        }
    };


    for (std::uint32_t y = tile_size; y != 0; ) {
        --y;
        if (rand() % 2) {
            for (std::uint32_t x = 0; x != tile_size; ++x) {
                inner(x, y);
            }
        }
        else {
            for (std::uint32_t x = tile_size; x != 0; ) {
                --x;
                inner(x, y);
            }
        }
    }

    std::for_each(d_pixels.begin(), d_pixels.end(), [](auto& p) { p.updated_this_frame = false; });
    for (std::size_t pos = 0; pos != tile_size * tile_size; ++pos) {
        d_buffer[pos] = d_pixels[pos].colour;
    }
}

auto tile::set(glm::ivec2 pos, const pixel& pixel) -> void
{
    assert(valid(pos));
    d_pixels[get_pos(pos)] = pixel;
}

auto tile::fill(const pixel& p) -> void
{
    d_pixels.fill(p);
}

auto tile::at(glm::ivec2 pos) const -> const pixel&
{
    return d_pixels[get_pos(pos)];
}

auto tile::at(glm::ivec2 pos) -> pixel&
{
    return d_pixels[get_pos(pos)];
}

auto tile::swap(glm::ivec2 lhs, glm::ivec2 rhs) -> glm::ivec2
{
    std::swap(at(lhs), at(rhs));
    return rhs;
}

}