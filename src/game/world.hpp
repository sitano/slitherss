#ifndef SLITHER_GAME_WORLD_HPP
#define SLITHER_GAME_WORLD_HPP

#include "snake.hpp"
#include "sector.hpp"
#include <memory>
#include <cstdint>
#include <unordered_map>

class world {
public:

    void init();
    void tick(long dt);

    snake::ptr create_snake();

    void init_random();
    int next_random();
    float next_randomf();
    template <typename T> T next_random(T base);

    typedef std::unordered_map<snake::snake_id_t, std::shared_ptr<snake>> snakes;

    void add_snake(snake::ptr ptr);
    void remove_snake(snake::snake_id_t id);
    snakes::iterator get_snake(snake::snake_id_t id);

    std::vector<snake *>& get_changes();

    // before calling this, snake must be flushed()
    void flush_changes(snake::snake_id_t id);
    // before calling this, all snakes must be flushed()
    void flush_changes();

    // world
    const uint16_t game_radius = 21600;
    const uint16_t max_snake_parts = 411;
    const uint16_t sector_size = 300;
    const uint16_t sector_count_along_edge = 144;

    // const
    static const long virtual_frame_time_ms = 8;

    static const uint8_t protocol_version = 8;

    static constexpr float f_pi = 3.14159265358979323846f;
    static constexpr float f_2pi = 2.0f * f_pi;

private:

    void tick_snakes(long dt);

private:
    // todo: reserve to collections
    snakes m_snakes;
    std::vector<sector> m_sectors;
    std::vector<snake *> m_changes;

    // todo fixed point arithmetic
    // todo pools
    // todo sorted checker

    // todo manage overflow, reuse old?
    uint16_t m_lastSnakeId = 0;
    long m_ticks = 0;
    uint32_t m_virtual_frames = 0;
};

#endif //SLITHER_GAME_WORLD_HPP
