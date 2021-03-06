#include "update_functions.h"
#include "utility.hpp"
#include "config.hpp"

#include <array>
#include <utility>
#include <variant>
#include <algorithm>
#include <random>

#include <glm/glm.hpp>

namespace sand {
namespace {

auto below(glm::ivec2 pos) -> glm::ivec2
{
    return pos + glm::ivec2{0, 1};
}

auto can_pixel_move_to(const tile& pixels, glm::ivec2 src_pos, glm::ivec2 dst_pos) -> bool
{
    if (!tile::valid(src_pos) || !tile::valid(dst_pos)) { return false; }

    // If the destination is empty, we can always move there
    if (pixels.at(dst_pos).type == pixel_type::none) { return true; }

    const auto& src = pixels.at(src_pos).properties().movement;
    const auto& dst = pixels.at(dst_pos).properties().movement;

    using pm = pixel_movement;
    switch (src) {
        case pm::solid:
            return dst == pm::liquid
                || dst == pm::gas;

        case pm::liquid:
            return dst == pm::gas;

        default:
            return false;
    }
}

auto set_adjacent_free_falling(tile& pixels, glm::ivec2 pos) -> void
{
    const auto l = pos + glm::ivec2{-1, 0};
    const auto r = pos + glm::ivec2{1, 0};

    if (pixels.valid(l)) {
        auto& px = pixels.at(l);
        const auto& props = px.properties();
        if (px.properties().movement == pixel_movement::solid) {
            px.is_falling = random_from_range(0.0f, 1.0f) > props.inertial_resistance || px.is_falling;
        }
    }

    if (pixels.valid(r)) {
        auto& px = pixels.at(r);
        const auto& props = px.properties();
        if (props.movement == pixel_movement::solid) {
            px.is_falling = random_from_range(0.0f, 1.0f) > props.inertial_resistance || px.is_falling;
        }
    }
}

auto move_towards(tile& pixels, glm::ivec2 from, glm::ivec2 offset) -> glm::ivec2
{
    glm::ivec2 curr_pos = from;

    const auto a = from;
    const auto b = from + offset;
    const auto steps = glm::max(glm::abs(a.x - b.x), glm::abs(a.y - b.y));

    for (int i = 0; i != steps; ++i) {
        const auto next_pos = a + (b - a) * (i + 1)/steps;

        if (!can_pixel_move_to(pixels, curr_pos, next_pos)) {
            break;
        }

        curr_pos = pixels.swap(curr_pos, next_pos);
        set_adjacent_free_falling(pixels, curr_pos);
    }

    if (curr_pos != from) {
        pixels.at(curr_pos).is_updated = true;
    }

    return curr_pos;
}

auto affect_neighbours(tile& pixels, glm::ivec2 pos) -> void
{
    const auto offsets = std::array{
        glm::ivec2{1, 0},
        glm::ivec2{-1, 0},
        glm::ivec2{0, 1},
        glm::ivec2{0, -1},
        glm::ivec2{1, 1},
        glm::ivec2{-1, -1},
        glm::ivec2{-1, 1},
        glm::ivec2{1, -1}
    };

    auto& pixel = pixels.at(pos);
    const auto& props = pixel.properties();

    const bool can_produce_embers = props.is_ember_source || pixel.is_burning;

    for (const auto& offset : offsets) {
        if (pixels.valid(pos + offset)) {
            auto& neighbour = pixels.at(pos + offset);

            // 1) Boil water
            if (props.can_boil_water) {
                if (neighbour.type == pixel_type::water) {
                    neighbour = pixel::steam();
                }
            }

            // 2) Corrode neighbours
            if (props.is_corrosion_source) {
                if (random_from_range(0.0f, 1.0f) > neighbour.properties().corrosion_resist) {
                    neighbour = pixel::air();
                    if (random_from_range(0.0f, 1.0f) > 0.9f) {
                        pixel = pixel::air();
                    }
                }
            }
            
            // 3) Spread fire
            if (props.is_burn_source || pixel.is_burning) {
                if (random_from_range(0.0f, 1.0f) < neighbour.properties().flammability) {
                    neighbour.is_burning = true;
                }
            }

            // 4) Produce embers
            if (can_produce_embers && neighbour.type == pixel_type::none) {
                if (random_from_range(0.0f, 1.0f) < 0.01f) {
                    neighbour = pixel::ember();
                }
            }
        }
    }
}

auto is_surrounded(tile& pixels, glm::ivec2 pos) -> bool
{
    const auto offsets = std::array{
        glm::ivec2{1, 0},
        glm::ivec2{-1, 0},
        glm::ivec2{0, 1},
        glm::ivec2{0, -1},
        glm::ivec2{1, 1},
        glm::ivec2{-1, -1},
        glm::ivec2{-1, 1},
        glm::ivec2{1, -1}
    };

    for (const auto& offset : offsets) {
        if (pixels.valid(pos + offset)) {
            auto& neighbour = pixels.at(pos + offset);

            if (neighbour.type == pixel_type::none) {
                return false;
            }
        }
    }

    return true;
}

enum class direction
{
    up, down
};

// Attempts to move diagonally up/down, and failing that, disperses outwards according
// to the dispersion rate
auto move_disperse(tile& pixels, glm::ivec2 pos, direction dir) -> glm::ivec2
{
    auto& data = pixels.at(pos);
    const auto& props = data.properties();

    auto offsets = std::array{
        glm::ivec2{-1, dir == direction::down ? 1 : -1},
        glm::ivec2{1,  dir == direction::down ? 1 : -1},
        glm::ivec2{-1 * props.dispersion_rate, 0},
        glm::ivec2{props.dispersion_rate, 0}
    };

    if (coin_flip()) std::swap(offsets[0], offsets[1]);
    if (coin_flip()) std::swap(offsets[2], offsets[3]);

    for (auto offset : offsets) {
        if (offset.y == 0) {
            data.velocity = {0.0, 0.0};
        }
        if (const auto new_pos = move_towards(pixels, pos, offset); new_pos != pos) {
            return new_pos;
        }
    }

    return pos;
}

}


auto update_movable_solid(tile& pixels, glm::ivec2 pos) -> glm::ivec2
{
    const auto original_pos = pos;
    const auto scope = scope_exit{[&] {
        pixels.at(pos).is_falling = pos != original_pos;
    }};

    // Apply gravity if can move down
    if (can_pixel_move_to(pixels, pos, below(pos))) {
        auto& vel = pixels.at(pos).velocity;
        vel += config::gravity * config::time_step;
        vel.y = glm::max(1.0f, vel.y);
        
        pos = move_towards(pixels, pos, vel);
    }

    // Transfer to horizontal
    else {
        auto& data = pixels.at(pos);
        auto& vel = data.velocity;
        if (vel.y > 5.0 && vel.x == 0.0) {
            const auto ht = data.properties().horizontal_transfer;
            vel.x = random_from_range(std::max(0.0f, ht - 0.1f), std::min(1.0f, ht + 0.1f)) * vel.y * sign_flip();
            vel.y = 0.0;
        }
        vel.x *= 0.8;

        pos = move_towards(pixels, pos, {vel.x, 0});
    }

    if (!pixels.at(pos).is_updated && pixels.at(pos).is_falling) {
        auto offsets = std::array{ glm::ivec2{-1, 1}, glm::ivec2{1, 1}, };
        if (coin_flip()) std::swap(offsets[0], offsets[1]);

        for (auto offset : offsets) {
            pos = move_towards(pixels, pos, offset);
            if (pos != original_pos) {
                break;
            }
        }
    }

    return pos;
}

auto update_liquid(tile& pixels, glm::ivec2 pos) -> glm::ivec2
{
    auto& vel = pixels.at(pos).velocity;
    vel += config::gravity * config::time_step;
    
    const auto offset = glm::ivec2{0, glm::max(1, (int)vel.y)};
    if (const auto new_pos = move_towards(pixels, pos, offset); new_pos != pos) {
        return new_pos;
    }

    return move_disperse(pixels, pos, direction::down);
}

auto update_gas(tile& pixels, glm::ivec2 pos) -> glm::ivec2
{
    auto& vel = pixels.at(pos).velocity;
    vel -= config::gravity * config::time_step;

    const auto offset = glm::ivec2{0, glm::min(-1, (int)vel.y)};
    if (const auto new_pos = move_towards(pixels, pos, offset); new_pos != pos) {
        return new_pos;
    }

    return move_disperse(pixels, pos, direction::up);
}

auto update_pixel(tile& pixels, glm::ivec2 pos) -> void
{
    if (pixels.at(pos).type == pixel_type::none) {
        return;
    }

    switch (pixels.at(pos).properties().movement) {
        case pixel_movement::solid: {
            pos = update_movable_solid(pixels, pos);
        } break;

        case pixel_movement::liquid: {
            pos = update_liquid(pixels, pos);
        } break;

        case pixel_movement::gas: {
            pos = update_gas(pixels, pos);
        } break;

        default: {

        } break;
    }

    affect_neighbours(pixels, pos);

    // Update logic for single pixels depending on properties only
    auto& pixel = pixels.at(pos);

    // is_burning status
    if (pixel.is_burning) {
        const auto& props = pixel.properties();

        // First, see if it can be put out
        if (is_surrounded(pixels, pos)) {
            if (random_from_range(0.0f, 1.0f) < props.put_out_surrounded) {
                pixel.is_burning = false;
            }
        } else {
            if (random_from_range(0.0f, 1.0f) < props.put_out) {
                pixel.is_burning = false;
            }
        }

        // Second, see if it gets destroyed
        if (random_from_range(0.0f, 1.0f) < props.burn_out_chance) {
            pixel = pixel::air();
        }
    }
}

}