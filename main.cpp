#include <utility.h>
#include <iostream>
#include <deque>
#include <tuple>
#include <vector>
#include <fstream>

int main() {

    // System configuration 
    const auto DISPATCHER_THRESHOLD = 2;
    const auto NUM_SERVER = 6;

    // System initiation 
    auto master_clock = float(0);
    auto dispatcher = Dispatcher(DISPATCHER_THRESHOLD);
    auto server_controller = ServerController(NUM_SERVER);
    auto permenent_depature = std::vector<Job>{}; // Store the permanent depatured jobs 
    auto complete_entry = std::vector<std::tuple<double, double, unsigned, unsigned>>{}; // Store the records 

    // Read data from file 
    auto filename = "config/interarrival_1.txt";
    auto s = std::ifstream(filename);
 
    if (!s.is_open()) {
        std::cout << "failed to open " << filename << '\n';
    } else {
        // std::string line;
        double arrival_time;
        while (s >> arrival_time) {
            std::cout << arrival_time << std::endl;
        }
        s.close();
    }

    return 0;
}
