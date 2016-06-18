#ifndef SLITHER_PACKET_FOOD_HPP
#define SLITHER_PACKET_FOOD_HPP

#include "p_base.hpp"
#include "game/food.hpp"

#include <vector>

// Sent when food that existed before enters range.
// The food id is calculated with (y * GameRadius * 3) + x
struct packet_set_food : public packet_base {
    explicit packet_set_food(std::vector<food> *ptr) : packet_base(packet_t_set_food), food_ptr(ptr) {}

    /**
     * 3	int8	Color?
     * 4-5	int16	Food X
     * 6-7	int16	Food Y
     * 8	int8	value / 5 -> Size
     */
    std::vector<food> *food_ptr;

    size_t get_size() const noexcept { return 3 + food_ptr->size() * 6; }
};

// Sent when food is created while in range (because of turbo or the death of a snake).
// The food id is calculated with (y * GameRadius * 3) + x
struct packet_spawn_food : public packet_base {
    packet_spawn_food() : packet_base(packet_t_spawn_food) {}

    /**
     * 3	int8	Color?
     * 4-5	int16	Food X
     * 6-7	int16	Food Y
     * 8	int8	value / 5 -> Size
     */
    food m_food;

    size_t get_size() const noexcept { return 3 + 6; }
};

// Sent when natural food spawns while in range.
// The food id is calculated with (y * GameRadius * 3) + x
struct packet_add_food : public packet_base {
    packet_add_food() : packet_base(packet_t_add_food) {}

    /**
     * 3	int8	Color?
     * 4-5	int16	Food X
     * 6-7	int16	Food Y
     * 8	int8	value / 5 -> Size
     */
    food m_food;

    size_t get_size() const noexcept { return 3 + 6; }
};

struct packet_eat_food : public packet_base {
    packet_eat_food() : packet_base(packet_t_eat_food) {}

    /**
     * 3-4	int16	Food X
     * 5-6	int16	Food Y
     */
    food m_food;

    // 7-8	int16	Eater snake id
    uint16_t snakeId = 0;

    size_t get_size() const noexcept { return 3 + 6; }
};

std::ostream& operator<<(std::ostream & out, const packet_set_food & p);
std::ostream& operator<<(std::ostream & out, const packet_spawn_food & p);
std::ostream& operator<<(std::ostream & out, const packet_add_food & p);
std::ostream& operator<<(std::ostream & out, const packet_eat_food & p);

#endif //SLITHER_PACKET_FOOD_HPP
