#include <vector>
#include <cassert>
#include <algorithm>
#include <utility>
#include <unordered_map>

#include "utils.h"

/* ---------------- Bit operations ---------------- */

int num_ones(int n) {
    int res = 0;
    while (n) {
        res += 1;
        n = n & (n - 1);
    }
    return res;
}

int vector_to_binary(std::vector<int>& v) {
    int res = 0;
    for (int elem : v) {
        res |= (1 << elem);
    }
    return res;
}

std::vector<int> binary_to_vector(int binary) {
    std::vector<int> res;
    int curr = 0;
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

int expand_deck(int hand, int deck) {
    int res = 0;
    int i = 0;
    while (deck) {
        if (not (hand & 1)) {
            res |= ((deck & 1) << i);
            deck >>= 1;
        }
        hand >>= 1;
        i += 1;
    }

    return res;
}

int compactify_deck(int hand, int deck) {
    int res = 0;
    int i = 0;
    while (deck) {
        if (not (hand & 1)) {
            res |= ((deck & 1) << i);
            i += 1;
        }
        else {
            assert(not (deck & 1));
        }
        deck >>= 1;
        hand >>= 1;
    }
    return res;
}

/* ---------------- State factorization ---------------- */

int masks[3] = {(1 << 8) - 1, ((1 << 8) - 1) << 8, ((1 << 8) - 1) << 16};

int apply_permutation(int num, int p1, int p2, int p3) {
    // x * 8 == x << 3
    return (
            ((num & masks[p1]) >> (p1 << 3)) |
            (((num & masks[p2]) >> (p2 << 3)) << 8) |
            (((num & masks[p3]) >> (p3 << 3)) << 16)
    );
}

int permutations[15] = {
    0, 2, 1,
    1, 0, 2,
    1, 2, 0,
    2, 0, 1,
    2, 1, 0
};

State canonicalize(State state) {
    int chand = state.hand;
    int cdeck = state.deck;
    for (int i = 0; i < 15; i += 3) {
        int p1 = permutations[i];
        int p2 = permutations[i + 1];
        int p3 = permutations[i + 2];

        int nhand = apply_permutation(state.hand, p1, p2, p3);
        int ndeck = apply_permutation(state.deck, p1, p2, p3);

        // The following `if` is simply this line unrolled:
        // chand, cdeck = min((chand, cdeck), (nhand, ndeck))
        if ((nhand < chand) or (nhand == chand and ndeck < cdeck)) {
            chand = nhand;
            cdeck = ndeck;
        }
    }
    return State {state.score, chand, cdeck};
}

inline bool eq3(int v1, int v2, int v3) {
    return (v1 == v2) and (v2 == v3);
}

int score_combination(std::vector<int>& combo) {
    std::vector<int> rem(combo.size());
    for (int i = 0; i < combo.size(); i++) {
        rem[i] = combo[i] % 8;
    }
    std::sort(rem.begin(), rem.end());

    if (eq3(combo[0] / 8, combo[1] / 8, combo[2] / 8) and eq3(rem[0], rem[1] - 1, rem[2] - 2)) {
        // same color, consecutive
        return (rem[0]) * 10 + 50;
    }
    else if (eq3(rem[0], rem[1], rem[2])) {
        // same numbers
        return (rem[0]) * 10 + 20;
    }
    else if (eq3(rem[0], rem[1] - 1, rem[2] - 2)) {
        // different colors, consecutive
        return (rem[0]) * 10 + 10;
    }
    else {
        throw "Tried to score a non-combination";
    }
}

std::vector<std::pair<int, std::vector<int>>> card_combinations(std::vector<int> hand) {
    std::vector<std::pair<int, std::vector<int>>> result;

    std::vector<std::vector<int>> v2c(8);
    for (int card : hand) {
        v2c[card % 8].push_back(card / 8);
    }

    // different colors, consecutive
    for (int v = 0; v < 6; v++) {
        if (!v2c[v].empty() and !v2c[v + 1].empty() and !v2c[v + 2].empty()) {
            for (int col1 : v2c[v]) {
                for (int col2 : v2c[v + 1]) {
                    for (int col3 : v2c[v + 2]) {
                        int score = eq3(col1, col2, col3) ? v * 10 + 50 : v * 10 + 10;
                        std::vector<int> combo = {v + col1 * 8, v + 1 + col2 * 8, v + 2 + col3 * 8};
                        assert(score_combination(combo) == score);
                        result.emplace_back(score, combo);
                    }
                }
            }
        }
    }

    // same numbers
    for (int v = 0; v < v2c.size(); v++) {
        std::vector<int> colors = v2c[v];
        if (colors.size() == 3) {
            int score = v * 10 + 20;
            std::vector<int> combo = {v + colors[0] * 8, v + colors[1] * 8, v + colors[2] * 8};
            assert(score_combination(combo) == score);
            result.emplace_back(score, combo);
        }
    }

    return result;
};

/* ---------------- Game moves ---------------- */

std::vector<Move> moves_from_hand(int hand) {
    // Order matters. The most promising moves should go first.
    std::vector<int> hand_as_vector = binary_to_vector(hand);
    // Throwing away the least valuable cards first.
    std::sort(hand_as_vector.begin(), hand_as_vector.end(),
        [](const int a, const int b) -> bool { return a % 8 < b % 8; });

    std::vector<Move> remove_moves;
    for (int i : hand_as_vector) {
        remove_moves.push_back(Move {0, (1 << i), 0});
    }

    std::vector<Move> deal_moves;
    for (std::pair<int, std::vector<int>> p : card_combinations(hand_as_vector)) {
        int score = p.first;
        std::vector<int> combo = p.second;
        deal_moves.push_back(Move {1, vector_to_binary(combo), score / 10});
    }

    // Dealing the most valuable combinations first.
    std::sort(deal_moves.begin(), deal_moves.end(),
        [](const Move& a, const Move& b) -> bool { return a.score_change > b.score_change; });

    std::vector<Move> moves;
    moves.insert(moves.end(), deal_moves.begin(), deal_moves.end());
    moves.insert(moves.end(), remove_moves.begin(), remove_moves.end());
    return moves;
}

//int best_move_score(int hand) {
//    std::vector<int> hand_as_vector = binary_to_vector(hand);
//    int best_score = 0;
//    for (std::pair<int, std::vector<int>> p : card_combinations(hand_as_vector)) {
//        int score = p.first;
//        best_score = std::max(best_score, score);
//    }
//    return best_score / 10;
//}

// ---------------- Cached stuff ----------------

// via https://stackoverflow.com/a/1617797/1214547
template <typename Iterator>
bool next_combination(const Iterator first, Iterator k, const Iterator last)
{
    /* Credits: Mark Nelson http://marknelson.us */
    if ((first == last) || (first == k) || (last == k))
        return false;
    Iterator i1 = first;
    Iterator i2 = last;
    ++i1;
    if (last == i1)
        return false;
    i1 = last;
    --i1;
    i2 = k;
    --i2;
    while (first != i1)
    {
        if (*--i1 < *i2)
        {
            Iterator j = k;
            while (!(*i1 < *j)) ++j;
            std::iter_swap(i1, j);
            ++i1;
            ++j;
            i2 = k;
            std::_V2::rotate(i1, j, last);
            while (last != j)
            {
                ++j;
                ++i2;
            }
            std::_V2::rotate(k, i2, last);
            return true;
        }
    }
    std::_V2::rotate(first, k, last);
    return false;
}

std::vector<int> make_hands(std::size_t hand_size) {
    std::vector<int> hands;

    std::vector<int> all_cards;
    for (int i = 0; i < 24; i++) {
        all_cards.push_back(i);
    }
    do {
        std::vector<int> hand_as_vector(all_cards.begin(), all_cards.begin() + hand_size);
        int hand = vector_to_binary(hand_as_vector);
        hands.push_back(hand);
    } while (next_combination(all_cards.begin(), all_cards.begin() + hand_size, all_cards.end()));

    return hands;
}

std::vector<int> hands5 = make_hands(5);

std::unordered_map<int, std::vector<Move>> make_moves_cache() {
    std::unordered_map<int, std::vector<Move>> moves_cache;
    for (int hand : hands5) {
        moves_cache[hand] = moves_from_hand(hand);
    }
    return moves_cache;
}

std::vector<State> make_starting_states() {
    std::vector<State> result;
    for (int hand : hands5) {
        int compact_deck = (1 << 19) - 1;
        int deck = expand_deck(hand, compact_deck);
        result.push_back(State {0, hand, deck});
    }
    return result;
}
//std::vector<int> hands4 = make_hands(4);
//std::vector<int> hands3 = make_hands(3);
//
//std::vector<int> make_all_hands() {
//    std::vector<int> result;
//    result.insert(result.end(), hands5.begin(), hands5.end());
//    result.insert(result.end(), hands4.begin(), hands4.end());
//    result.insert(result.end(), hands3.begin(), hands3.end());
//    return result;
//}
//
//std::vector<int> all_hands = make_all_hands();

;

// (full hand as a bitmask) -> (list of Move objects)
std::unordered_map<int, std::vector<Move>> moves_cache = make_moves_cache();

//std::unordered_map<int, int> make_score_change_cache() {
//    std::unordered_map<int, int> result;
//    for (int hand : all_hands) {
//        result[hand] = best_move_score(hand);
//    }
//    return result;
//};
//
//std::unordered_map<int, int> score_change_cache = make_score_change_cache();

/* ---------------- More game moves ---------------- */

std::vector<Move> moves(State state) {
    assert(state.deck);
    return moves_cache[state.hand];
}

std::vector<State> outcomes(State state, Move move) {
    assert(state.deck);
    assert((state.hand & move.param) == move.param);
    int new_score = state.score + move.score_change;
    int new_hand_partial = state.hand ^ move.param;
    std::vector<State> result;

    if (move.action == 0) {
        // remove
        int temp = state.deck;
        int card = 0;
        while (temp) {
            if (temp & 1) {
                int new_hand = new_hand_partial | (1 << card);
                int new_deck = state.deck ^ (1 << card);
                result.push_back(State {new_score, new_hand, new_deck});
            }
            card += 1;
            temp >>= 1;
        }
    }
    else {
        // deal
        // the slow part BEGINS
        std::vector<int> deck_vector = binary_to_vector(state.deck);
        int replenishment = std::min(static_cast<int>(deck_vector.size()), 3);

        do {
            std::vector<int> combo(deck_vector.begin(), deck_vector.begin() + replenishment);
            int mask = vector_to_binary(combo);
            int new_hand = new_hand_partial | mask;
            int new_deck = state.deck ^ mask;
            result.push_back(State {new_score, new_hand, new_deck});
        } while (next_combination(deck_vector.begin(), deck_vector.begin() + replenishment, deck_vector.end()));
        // the slow part ENDS
    }
    return result;
}

template<typename T>
void mark_states_reachable_from(State state, T& storage) {
    if (state.score >= 40) {
        return;
    }
    else if (not state.deck) {
        return;
    }
    else {
        assert(not storage[state]);
        storage[state] = true;

        for (Move& move : moves(state)) {
            std::vector<State> curr_outcomes = outcomes(state, move);
            auto num_outcomes = static_cast<int>(curr_outcomes.size());
            assert(num_outcomes > 0);

            for (State& outcome : curr_outcomes) {
                if (not storage[outcome]) {
                    mark_states_reachable_from(outcome, storage);
                }
            }
        }
    }
}

