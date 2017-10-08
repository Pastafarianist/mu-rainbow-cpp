#ifndef CPP_UTILS_H
#define CPP_UTILS_H

#include <vector>
#include <cassert>

/* ---------------- Datatypes ---------------- */

struct State {
    uint16_t score;
    uint32_t hand, deck;
};

struct Move {
    uint32_t action, param;
    uint16_t score_change;
};

/* ------ Kind of private but not really ------*/

uint32_t num_ones(uint32_t n);

std::vector<Move> moves(State state);

std::vector<State> outcomes(State state, Move move);

/* ------------ Exported functions ------------*/

std::vector<uint32_t> binary_to_vector(uint32_t binary);

State canonicalize(State state);

uint32_t compactify_deck(uint32_t hand, uint32_t deck);

std::vector<uint32_t> make_hands(std::size_t hand_size);

bool is_storable(State state);

template<typename T>
void mark_states_reachable_from(State state, T &storage) {
    if (not is_storable(state)) {
        return;
    } else {
        assert(not storage[state]);
        storage[state] = true;

        for (Move &move : moves(state)) {
            std::vector<State> curr_outcomes = outcomes(state, move);
            assert(not curr_outcomes.empty());

            for (State &outcome : curr_outcomes) {
                if (is_storable(outcome) and (not storage[outcome])) {
                    mark_states_reachable_from(outcome, storage);
                }
            }
        }
    }
}

std::vector<State> make_starting_states();

#endif //CPP_UTILS_H
