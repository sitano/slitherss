#ifndef SLITHER_PACKET_PREY_HPP
#define SLITHER_PACKET_PREY_HPP

#include "p_base.hpp"

// Sent when a Prey enters range / spawns or leaves range / gets eaten. The exact event/format depends on the packet-length:
struct packet_add_prey : public packet_base {
    packet_add_prey() : packet_base(packet_t_add_prey) { }

    /**
     * packet-length 22: add Prey

     *  Bytes	Data type	Description
     *  3-4	int16	Prey ID
     *  5	int8	color ID
     *  6-8	int24	value / 5 -> x
     *  9-11	int24	value / 5 -> y
     *  12	int8	value / 5 -> size
     *  13	int8	value - 48 -> direction (see packet "j")
     *  14-16	int24	value * 2*PI / 16777215 -> wanted angle
     *  17-19	int24	value * 2*PI / 16777215 -> current angle
     *  20-21	int16	value / 1000 -> speed
     */

    // TODO ...
};

// Sent when prey ("flying particles") is updated.
struct packet_update_prey : public packet_base {
    packet_update_prey() : packet_base(packet_t_mov_prey) {}

    uint16_t preyId; // 3-4	int16	Prey ID
    uint16_t x; // 5-6	int16	value * 3 + 1 -> x
    uint16_t y; // 7-8	int16	value * 3 + 1 -> y

    // Next follow updates for one or more of these variables:
    uint8_t direction; // int8	value - 48 -> direction (0: not turning; 1: turning counter-clockwise; 2: turning clockwise)
    uint24_t angle; // int24	value * 2*PI / 16777215 -> current angle
    uint24_t wangle; // int24	value * 2*PI / 16777215 -> wanted angle (angle the prey turns towards)
    float speed; // int16	value / 1000 -> speed

    // Depending on the packet-length, different variables are sent:
    // packet-length	variables sent (in that exact order)
    // 11	speed
    // 12	current angle
    // 13	direction, wanted angle
    // 14	current angle, speed
    // 15	direction, wanted angle, speed
    // 16	direction, current angle, wanted angle
    // 18	direction, current angle, wanted angle, speed

    // TODO constexpr size_t get_size() const noexcept { return 3 + 6 + ...; }
};

// Sent when a Prey enters range / spawns or leaves range / gets eaten. The exact event/format depends on the packet-length:
struct packet_remove_prey : public packet_base {
    packet_remove_prey() : packet_base(packet_t_rem_prey) {}

    // packet-length 5: remove Prey
    uint16_t preyId; // 3-4	int16	Prey ID

    // packet-length 7: eat Prey
    uint16_t snakeId; // 5-6	int16	Eater snake ID

    // TODO constexpr size_t get_size() const noexcept { return ...; }
};

#endif //SLITHER_PACKET_PREY_HPP
