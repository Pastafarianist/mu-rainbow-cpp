#include <vector>
#include <cassert>
#include <algorithm>
#include <unordered_map>

#include "utils.h"

/* ---------------- Bit operations ---------------- */

uint32_t num_ones(uint32_t n) {
    uint32_t res = 0;
    while (n) {
        res += 1;
        n = n & (n - 1);
    }
    return res;
}

uint32_t vector_to_binary(std::vector<uint32_t> &v) {
    uint32_t res = 0;
    for (uint32_t elem : v) {
        res |= (1 << elem);
    }
    return res;
}

std::vector<uint32_t> binary_to_vector(uint32_t binary) {
    std::vector<uint32_t> res;
    uint32_t curr = 0;
    while (binary) {
        if (binary & 1) {
            res.push_back(curr);
        }
        curr += 1;
        binary >>= 1;
    }
    return res;
}

/* ---------------- Deck manipulations ---------------- */

uint32_t expand_deck(uint32_t hand, uint32_t deck) {
    uint32_t res = 0;
    uint32_t i = 0;
    while (deck) {
        if (not(hand & 1)) {
            res |= ((deck & 1) << i);
            deck >>= 1;
        }
        hand >>= 1;
        i += 1;
    }

    return res;
}

uint32_t compactify_deck(uint32_t hand, uint32_t deck) {
    uint32_t res = 0;
    uint32_t i = 0;
    while (deck) {
        if (not(hand & 1)) {
            res |= ((deck & 1) << i);
            i += 1;
        } else {
            assert(not(deck & 1));
        }
        deck >>= 1;
        hand >>= 1;
    }
    return res;
}

/* ---------------- State factorization ---------------- */

uint32_t masks[3] = {(1 << 8) - 1, ((1 << 8) - 1) << 8, ((1 << 8) - 1) << 16};

uint32_t apply_permutation(uint32_t num, uint32_t p1, uint32_t p2, uint32_t p3) {
    // x * 8 == x << 3
    return (
            ((num & masks[p1]) >> (p1 << 3)) |
            (((num & masks[p2]) >> (p2 << 3)) << 8) |
            (((num & masks[p3]) >> (p3 << 3)) << 16)
    );
}

uint32_t permutations[15] = {
        0, 2, 1,
        1, 0, 2,
        1, 2, 0,
        2, 0, 1,
        2, 1, 0
};

State canonicalize(State state) {
    uint32_t chand = state.hand;
    uint32_t cdeck = state.deck;
    for (std::size_t i = 0; i < 15; i += 3) {
        uint32_t p1 = permutations[i];
        uint32_t p2 = permutations[i + 1];
        uint32_t p3 = permutations[i + 2];

        uint32_t nhand = apply_permutation(state.hand, p1, p2, p3);
        uint32_t ndeck = apply_permutation(state.deck, p1, p2, p3);

        // The following `if` is simply this line unrolled:
        // chand, cdeck = min((chand, cdeck), (nhand, ndeck))
        if ((nhand < chand) or (nhand == chand and ndeck < cdeck)) {
            chand = nhand;
            cdeck = ndeck;
        }
    }
    return State {state.score, chand, cdeck};
}

inline bool eq3(uint32_t v1, uint32_t v2, uint32_t v3) {
    return (v1 == v2) and (v2 == v3);
}

uint32_t score_combination(std::vector<uint32_t> &combo) {
    std::vector<uint32_t> rem(combo.size());
    for (uint32_t i = 0; i < combo.size(); i++) {
        rem[i] = combo[i] % 8;
    }
    std::sort(rem.begin(), rem.end());

    if (eq3(combo[0] / 8, combo[1] / 8, combo[2] / 8) and eq3(rem[0], rem[1] - 1, rem[2] - 2)) {
        // same color, consecutive
        return (rem[0]) * 10 + 50;
    } else if (eq3(rem[0], rem[1], rem[2])) {
        // same numbers
        return (rem[0]) * 10 + 20;
    } else if (eq3(rem[0], rem[1] - 1, rem[2] - 2)) {
        // different colors, consecutive
        return (rem[0]) * 10 + 10;
    } else {
        throw "Tried to score a non-combination";
    }
}

std::vector<std::pair<uint16_t, std::vector<uint32_t>>> card_combinations(std::vector<uint32_t> hand) {
    std::vector<std::pair<uint16_t, std::vector<uint32_t>>> result;

    std::vector<std::vector<uint32_t>> v2c(8);
    for (uint32_t card : hand) {
        v2c[card % 8].push_back(card / 8);
    }

    // different colors, consecutive
    for (uint16_t v = 0; v < 6; v++) {
        if (!v2c[v].empty() and !v2c[v + 1].empty() and !v2c[v + 2].empty()) {
            for (uint32_t col1 : v2c[v]) {
                for (uint32_t col2 : v2c[v + 1]) {
                    for (uint32_t col3 : v2c[v + 2]) {
                        uint16_t score = eq3(col1, col2, col3) ? v * 10 + 50 : v * 10 + 10;
                        std::vector<uint32_t> combo = {v + col1 * 8, v + 1 + col2 * 8, v + 2 + col3 * 8};
                        assert(score_combination(combo) == score);
                        result.emplace_back(score, combo);
                    }
                }
            }
        }
    }

    // same numbers
    for (uint16_t v = 0; v < v2c.size(); v++) {
        std::vector<uint32_t> colors = v2c[v];
        if (colors.size() == 3) {
            uint16_t score = v * 10 + 20;
            std::vector<uint32_t> combo = {v + colors[0] * 8, v + colors[1] * 8, v + colors[2] * 8};
            assert(score_combination(combo) == score);
            result.emplace_back(score, combo);
        }
    }

    return result;
}

/* ---------------- Game moves ---------------- */

std::vector<Move> moves_from_hand(uint32_t hand) {
    // Order matters. The most promising moves should go first.
    std::vector<uint32_t> hand_as_vector = binary_to_vector(hand);
    // Throwing away the least valuable cards first.
    std::sort(hand_as_vector.begin(), hand_as_vector.end(),
              [](const uint32_t a, const uint32_t b) -> bool { return a % 8 < b % 8; });

    std::vector<Move> remove_moves;
    for (uint32_t i : hand_as_vector) {
        remove_moves.push_back(Move {0, (1u << i), 0});
    }

    std::vector<Move> deal_moves;
    for (std::pair<uint16_t, std::vector<uint32_t>> p : card_combinations(hand_as_vector)) {
        uint16_t score = p.first;
        std::vector<uint32_t> combo = p.second;
        deal_moves.push_back(Move {1, vector_to_binary(combo), score / static_cast<uint16_t>(10)});
    }

    // Dealing the most valuable combinations first.
    std::sort(deal_moves.begin(), deal_moves.end(),
              [](const Move &a, const Move &b) -> bool { return a.score_change > b.score_change; });

    std::vector<Move> moves;
    moves.insert(moves.end(), deal_moves.begin(), deal_moves.end());
    moves.insert(moves.end(), remove_moves.begin(), remove_moves.end());
    return moves;
}

//uint32_t best_move_score(uint32_t hand) {
//    std::vector<uint32_t> hand_as_vector = binary_to_vector(hand);
//    uint32_t best_score = 0;
//    for (std::pair<uint32_t, std::vector<uint32_t>> p : card_combinations(hand_as_vector)) {
//        uint32_t score = p.first;
//        best_score = std::max(best_score, score);
//    }
//    return best_score / 10;
//}

// ---------------- Cached stuff ----------------

class CombinationGen {
private:
    std::vector<bool> mask;
    std::size_t n_, k_;
    bool is_done;

public:
    CombinationGen(std::size_t n, std::size_t k) : n_(n), k_(k), is_done(false) {
        mask.resize(n);
        std::fill(mask.begin(), mask.begin() + k, true);
    }

    bool done() {
        return is_done;
    }

    std::vector<uint32_t> next() {
        if (is_done) {
            throw "Generator is exhausted";
        }

        std::vector<uint32_t> result;
        result.reserve(k_);

        for (uint32_t i = 0; i < n_; i++) {
            if (mask[i]) {
                result.push_back(i);
            }
        }

        is_done = not std::prev_permutation(mask.begin(), mask.end());
        return result;
    }
};

std::vector<uint32_t> make_hands(std::size_t hand_size) {
    std::vector<uint32_t> hands;

    CombinationGen gen(24, hand_size);
    do {
        std::vector<uint32_t> hand_as_vector = gen.next();
        uint32_t hand = vector_to_binary(hand_as_vector);
        hands.push_back(hand);
    } while (not gen.done());

    return hands;
}

std::vector<uint32_t> hands5 = make_hands(5);
//std::vector<uint32_t> hands4 = make_hands(4);
//std::vector<uint32_t> hands3 = make_hands(3);
//
//std::vector<uint32_t> make_all_hands() {
//    std::vector<uint32_t> result;
//    result.insert(result.end(), hands5.begin(), hands5.end());
//    result.insert(result.end(), hands4.begin(), hands4.end());
//    result.insert(result.end(), hands3.begin(), hands3.end());
//    return result;
//}
//
//std::vector<uint32_t> all_hands = make_all_hands();

std::unordered_map<uint32_t, std::vector<Move>> make_moves_cache() {
    std::unordered_map<uint32_t, std::vector<Move>> moves_cache;
    for (uint32_t hand : hands5) {
        moves_cache[hand] = moves_from_hand(hand);
    }
    return moves_cache;
}

// (full hand as a bitmask) -> (list of Move objects)
std::unordered_map<uint32_t, std::vector<Move>> moves_cache = make_moves_cache();

std::vector<State> make_starting_states() {
    std::vector<State> result;
    for (uint32_t hand : hands5) {
        uint32_t compact_deck = (1 << 19) - 1;
        uint32_t deck = expand_deck(hand, compact_deck);
        result.push_back(State {0, hand, deck});
    }
    return result;
}

//std::unordered_map<uint32_t, uint32_t> make_score_change_cache() {
//    std::unordered_map<uint32_t, uint32_t> result;
//    for (uint32_t hand : all_hands) {
//        result[hand] = best_move_score(hand);
//    }
//    return result;
//};
//
//std::unordered_map<uint32_t, uint32_t> score_change_cache = make_score_change_cache();

/* ---------------- More game moves ---------------- */

std::vector<Move> moves(State state) {
    assert(state.deck);
    return moves_cache[state.hand];
}

std::vector<State> outcomes(State state, Move move) {
    assert(state.deck);
    assert((state.hand & move.param) == move.param);
    uint16_t new_score = state.score + move.score_change;
    uint32_t new_hand_partial = state.hand ^ move.param;
    std::vector<State> result;

    if (move.action == 0) {
        // remove
        uint32_t temp = state.deck;
        uint32_t card = 0;
        while (temp) {
            if (temp & 1) {
                uint32_t new_hand = new_hand_partial | (1 << card);
                uint32_t new_deck = state.deck ^ (1 << card);
                result.push_back(State {new_score, new_hand, new_deck});
            }
            card += 1;
            temp >>= 1;
        }
    } else {
        // deal
        // the slow part BEGINS
        std::vector<uint32_t> deck_vector = binary_to_vector(state.deck);
        std::size_t replenishment = std::min(deck_vector.size(), static_cast<std::size_t>(3));

        CombinationGen gen(deck_vector.size(), replenishment);
        do {
            std::vector<uint32_t> combo_idx = gen.next();
            std::vector<uint32_t> combo;
            combo.reserve(replenishment);

            for (uint32_t idx : combo_idx) {
                combo.push_back(deck_vector[idx]);
            }

            uint32_t mask = vector_to_binary(combo);
            uint32_t new_hand = new_hand_partial | mask;
            uint32_t new_deck = state.deck ^ mask;
            result.push_back(State {new_score, new_hand, new_deck});
        } while (not gen.done());
        // the slow part ENDS
    }
    return result;
}

bool is_storable(State state) {
    return (state.score < 40) and (state.deck) and (num_ones(state.hand) == 5);
}