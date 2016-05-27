#ifndef SLITHER_SERVER_BUF_HPP
#define SLITHER_SERVER_BUF_HPP

template <size_t size> struct packet_size {
    static const size_t bytes = size;
};

#endif //SLITHER_SERVER_BUF_HPP
