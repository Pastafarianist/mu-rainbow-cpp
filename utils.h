//
// Created by ser on 05.10.17.
//

#ifndef CPP_UTILS_H
#define CPP_UTILS_H

#include <vector>
#include <cassert>

/* ---------------- Datatypes ---------------- */

struct State {
    uint32_t score, hand, deck;
};

struct Move {
    uint32_t action, param, score_change;
};

/* ------ Kind of private but not really ------*/

std::vector<Move> moves(State state);
std::vector<State> outcomes(State state, Move move);

/* ------------ Exported functions ------------*/

State canonicalize(State state);
uint32_t compactify_deck(uint32_t hand, uint32_t deck);

std::vector<uint32_t> make_hands(std::size_t hand_size);

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

std::vector<State> make_starting_states();

#endif //CPP_UTILS_H
