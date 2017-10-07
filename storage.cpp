#include "storage.h"
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <cassert>
#include <iostream>
#include <fstream>

using std::vector;

std::unordered_map<uint32_t, uint32_t> make_hands5_factor_rev() {
    vector<uint32_t> hands5 = make_hands(5);
    std::unordered_set<uint32_t> set_hands;
    for (uint32_t hand : hands5) {
        State cstate = canonicalize(State {0, hand, 0});
        uint32_t chand = cstate.hand;
        set_hands.insert(chand);
    }
    vector<uint32_t> hands5_factor(set_hands.begin(), set_hands.end());
    std::sort(hands5_factor.begin(), hands5_factor.end());

    std::unordered_map<uint32_t, uint32_t> result;
    for (uint32_t i = 0; i < hands5_factor.size(); ++i) {
        result[hands5_factor[i]] = i;
    }

    return result;
}

std::unordered_map<uint32_t, uint32_t> hands5_factor_rev = make_hands5_factor_rev();

std::size_t Storage::state_to_offset(State state) {
    State cstate = canonicalize(state);
    std::size_t idx = cstate.score;

//    vector<uint32_t> vhand = binary_to_vector(cstate.hand);
//    for (uint32_t card : vhand) {
//        std::cerr << card << ", ";
//    }
//    std::cerr << std::endl;

    assert(hands5_factor_rev.find(cstate.hand) != hands5_factor_rev.end());

    std::size_t hand_offset = hands5_factor_rev[cstate.hand];
    std::size_t deck_offset = compactify_deck(cstate.hand, cstate.deck);

    assert(hands5_factor_rev.size() == 7448);

    // Multiplication by 2**19.
    std::size_t offset = ((idx * 7448 + hand_offset) << 19) + deck_offset;
    return offset;
}

// https://stackoverflow.com/a/4666996/1214547
template<std::size_t I>
void Storage::bitset_dump(const std::bitset<I> &in, std::ostream &out) {
    // export a bitset consisting of I bits to an output stream.
    // Eight bits are stored to a single stream byte.
    std::size_t i = 0;  // the current bit index
    uint8_t c = 0; // the current byte
    uint8_t bits = 0;      // to process next byte
    while (i < in.size()) {
        c = c << 1;
        if (in[i]) ++c; // adding 1 if bit is true
        ++bits;
        if (bits == 8) {
            out.put(static_cast<char>(c));
            c = 0;
            bits = 0;
        }
        ++i;
    }
    // dump remaining
    if (bits != 0) {
        // pad the byte so that first bits are in the most significant positions.
        while (bits != 8) {
            c = c << 1;
            ++bits;
        }
        out.put(static_cast<char>(c));
    }
}

template<std::size_t I>
void Storage::bitset_restore(std::istream &in, std::bitset<I> &out) {
    // read bytes from the input stream to a bitset of size I.
    /* for debug */ //for(int n = 0; n < I; ++n) out.at(n) = false;
    unsigned int i = 0;          // current bit index
    unsigned char mask = 0x80;   // current byte mask
    unsigned char c = 0;         // current byte in stream
    while (in.good() && (i < I)) {
        if ((i % 8) == 0) { // retrieve next character
            c = static_cast<unsigned char>(in.get());
            mask = 0x80;
        } else mask = mask >> 1;   // shift mask
        out[i] = (c & mask);
        ++i;
    }
}

Storage::Proxy Storage::operator[](State state) {
    std::size_t idx = state_to_offset(state);
    return Proxy(&bs, idx);
}

void Storage::dump(std::string path) {
    std::ofstream f;
    f.open(path);
    bitset_dump(bs, f);
    f.close();
}
