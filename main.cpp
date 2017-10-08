#include <iostream>
#include <vector>
#include "storage.h"


int main() {
    auto *storage = new Storage();
    std::vector<State> starting_states = make_starting_states();
    for (std::size_t i = 0; i < starting_states.size(); i++) {
        std::cout << "Processing state " << (i + 1) << "/" << starting_states.size() << std::endl;
        mark_states_reachable_from(starting_states[i], *storage);
    }
    storage->dump("reachability.bin");
    delete storage;
    return 0;
}