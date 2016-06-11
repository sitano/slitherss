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
    void init_sectors();
    void init_food();

    void tick(long dt);

    snake::ptr create_snake();
    snake::ptr create_snake_bot();
    void spawn_snakes(const int count);
    void check_snake_bounds(snake *s);

    void init_random();
    int next_random();
    float next_randomf();
    template <typename T> T next_random(T base);

    typedef std::unordered_map<snake::snake_id_t, std::shared_ptr<snake>> snakes;
    typedef std::vector<snake::snake_id_t> v_ids;

    void add_snake(snake::ptr ptr);
    void remove_snake(snake::snake_id_t id);
    snakes::iterator get_snake(snake::snake_id_t id);
    snakes& get_snakes();
    v_ids& get_dead();

    std::vector<snake *>& get_changes();

    // before calling this, snake must be flushed()
    void flush_changes(snake::snake_id_t id);
    // before calling this, all snakes must be flushed()
    void flush_changes();

    // world
    const uint16_t game_radius = 21600;
    const uint16_t max_snake_parts = 411;
    const uint16_t sector_size = 300;
    const uint16_t sector_count_along_edge = 2 * game_radius / sector_size;
    const uint16_t death_radius = game_radius - sector_size;

    // const
    static const long virtual_frame_time_ms = 8;

    static const uint8_t protocol_version = 8;

    static constexpr float f_pi = 3.14159265358979323846f;
    static constexpr float f_2pi = 2.0f * f_pi;

private:

    void tick_snakes(long dt);
    void tick_bot(snake *s);

private:
    // todo: reserve to collections
    snakes m_snakes;
    v_ids m_dead;
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

std::ostream& operator<<(std::ostream & out, const world & w);

/**
 * http://stackoverflow.com/questions/563198/how-do-you-detect-where-two-line-segments-intersect
 *
 * FWIW, the following function (in C) both detects line intersections and determines the intersection point.
 * It is based on an algorithm in Andre LeMothe's "Tricks of the Windows Game Programming Gurus".
 * It's not dissimilar to some of the algorithm's in other answers (e.g. Gareth's).
 * LeMothe then uses Cramer's Rule (don't ask me) to solve the equations themselves.
 *
 * I can attest that it works in my feeble asteroids clone, and seems to deal correctly with the edge cases
 * described in other answers by Elemental, Dan and Wodzu. It's also probably faster than the code posted by
 * KingNestor because it's all multiplication and division, no square roots!
 */
bool intersect(float p0_x, float p0_y, float p1_x, float p1_y,
               float p2_x, float p2_y, float p3_x, float p3_y);

#endif //SLITHER_GAME_WORLD_HPP
