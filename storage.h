#ifndef CPP_STORAGE_H
#define CPP_STORAGE_H

#include <bitset>
#include <cstdint>
#include "utils.h"

const std::size_t bs_size = 40L * 7448L * (1L << 19);

class Storage {
private:
    std::bitset<bs_size> bs;

    std::size_t state_to_offset(State state);

    template<std::size_t I>
    void bitset_dump(const std::bitset<I> &in, std::ostream &out);

    template<std::size_t I>
    void bitset_restore(std::istream &in, std::bitset<I> &out);

    class Proxy {
    private:
        std::bitset<bs_size> *bs_;
        std::size_t idx_;

    public:

        operator bool() const {
            return (*bs_)[idx_];
        }

        bool operator=(bool v) {
            return (*bs_)[idx_] = v;
        }

    public:
        Proxy(std::bitset<bs_size> *bs, std::size_t idx) : bs_(bs), idx_(idx) {};
    };

public:
    Proxy operator[](State state);

    void dump(std::string path);
};

#endif //CPP_STORAGE_H
