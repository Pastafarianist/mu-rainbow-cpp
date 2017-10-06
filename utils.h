//
// Created by ser on 05.10.17.
//

#ifndef CPP_UTILS_H
#define CPP_UTILS_H

#include <vector>

/* ---------------- Datatypes ---------------- */

struct State {
    int score, hand, deck;
};

struct Move {
    int action, param, score_change;
};

/* ------------ Exported functions ------------*/

State canonicalize(State state);
int compactify_deck(int hand, int deck);

std::vector<int> make_hands(std::size_t hand_size);

template<typename T>
void mark_states_reachable_from(State state, T& storage);
std::vector<State> make_starting_states();

#endif //CPP_UTILS_H
