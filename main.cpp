#include <iostream>
#include <vector>
#include <memory>
#include "storage.h"


int main() {
    auto storage = std::make_unique<Storage>();
    std::vector<State> starting_states = make_starting_states();
    for (std::size_t i = 0; i < starting_states.size(); i++) {
        std::cout << "Processing state " << (i + 1) << "/" << starting_states.size() << std::endl;
        mark_states_reachable_from(starting_states[i], *storage);
    }
    const std::string dump_filename = "reachability.bin";
    std::cout << "Done! Dumping result to " << dump_filename << std::endl;
    storage->dump(dump_filename);
    return 0;
}