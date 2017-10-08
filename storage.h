#ifndef CPP_STORAGE_H
#define CPP_STORAGE_H

#include <bitset>
#include <cstdint>
#include <iostream>
#include "utils.h"

const std::size_t bs_size = 40L * 7448L * (1L << 19);

class Storage {
private:
    std::bitset<bs_size> bs;

    std::size_t total_use;
    std::vector<std::size_t> bucket_use;

    std::size_t state_to_offset(State state);

    template<std::size_t I>
    void bitset_dump(const std::bitset<I> &in, std::ostream &out);

    template<std::size_t I>
    void bitset_restore(std::istream &in, std::bitset<I> &out);

    class Proxy {
    private:
        Storage* parent_;
        std::size_t idx_;
        std::uint8_t bucket_idx_;

    public:

        operator bool() const {
            return (parent_->bs)[idx_];
        }

        bool operator=(bool v) {
            if ((not (parent_->bs)[idx_]) and v) {
                ++parent_->total_use;
                ++parent_->bucket_use[bucket_idx_];
                if (parent_->total_use % 10000000 == 0) {
                    std::cout << "total_use == " << parent_->total_use << std::endl;
                    std::cout << "    bucket_use == [" << std::endl;
                    for (std::size_t i = 0; i < parent_->bucket_use.size(); ++i) {
                        std::cout << "        " << i << ": "
                                  << parent_->bucket_use[i] << "," << std::endl;
                    }
                    std::cout << "    ]" << std::endl;
                }
            }
            return (parent_->bs)[idx_] = v;
        }

    public:
        Proxy(Storage* parent, std::size_t idx, std::uint8_t bucket_idx)
                : parent_(parent), idx_(idx), bucket_idx_(bucket_idx) {};
    };

public:
    Storage();

    Proxy operator[](State state);

    void dump(std::string path);
};

#endif //CPP_STORAGE_H
