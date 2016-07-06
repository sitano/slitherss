#ifndef SLITHER_PACKET_HIGHSCORE_HPP
#define SLITHER_PACKET_HIGHSCORE_HPP

#include "p_base.h"
#include "game/snake.h"

#include <memory>

struct packet_highscore : public packet_base {
    packet_highscore() : packet_base(packet_t_highscore) {}

    // 3-5	int24	J (for snake length calculation)
    // 6-8	int24	I (for snake length calculation; value / 16777215)
    // 9	int8	The length of the winners name
    // 10-?	string	Winners name
    std::shared_ptr<snake> winner;

    // ?-?	string	Winners message
    std::string message;

    size_t get_size() const noexcept {
        return 9 + 1 + winner->name.length() + message.length();
    }
};

std::ostream& operator<<(std::ostream & out, const packet_highscore & p);

#endif //SLITHER_PACKET_HIGHSCORE_HPP
