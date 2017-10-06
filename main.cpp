#include <iostream>
#include <vector>
#include "storage.h"
#include "utils.h"


int main() {
    Storage storage;
    std::vector<State> starting_states = make_starting_states();
    for (int i = 0; i < starting_states.size(); i++) {
        std::cout << "Processing state " << (i + 1) << "/" << starting_states.size() << std::endl;
        mark_states_reachable_from(starting_states[i], storage);
    }
    return 0;
}