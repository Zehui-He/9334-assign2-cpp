#include <utility.h>
#include <iostream>
#include <deque>
#include <tuple>
#include <vector>
#include <sstream>

using Next_event = bool;
constexpr bool ARRIVAL = true;
constexpr bool DEPARTURE = false;

int main(int argc, char* argv[]) {
    auto test_idx = argv[1]; // should receive from args 

    // System configuration 
    std::stringstream para_filename;
    para_filename << "config/para_" << test_idx << ".txt";
    auto [NUM_SERVER, DISPATCHER_THRESHOLD] = read_para(para_filename.str());
    

    // System initiation 
    auto master_clock = float(0);
    auto dispatcher = Dispatcher(DISPATCHER_THRESHOLD);
    auto server_controller = ServerController(NUM_SERVER);
    auto permenent_depature = std::vector<Job>{}; // Store the permanent depatured jobs 
    auto complete_entry = std::vector<std::tuple<double, double, unsigned, unsigned>>{}; // Store the records 

    // Create file paths 
    std::stringstream inter_filename;
    std::stringstream procs_filename;
    inter_filename << "config/interarrival_" << test_idx << ".txt";
    procs_filename << "config/service_" << test_idx << ".txt";
    
    // Read data from file 
    auto inter_arrival_list = read_inter_arrival(inter_filename.str());
    auto procs_time_list = read_processing_time(procs_filename.str());

    // Zip two files into one 
    auto job_data = zip(std::move(inter_arrival_list), std::move(procs_time_list));

    while (job_data.size() != 0 || server_controller.server_busy()) {
        // Find the next depature and next arrival time 
        auto [first_departure_time, first_departure_server] = server_controller.first_departure_time_server();
        double next_arrival_time;
        if (job_data.size() == 0) {
            next_arrival_time = FLOAT_INF;
        } else {
            next_arrival_time = job_data[0].first;
        }
        
        // Determine the next event 
        double next_event_time;
        Next_event next_event_type;
        if (next_arrival_time < first_departure_time) {
            next_event_time = next_arrival_time;
            next_event_type = ARRIVAL;
        }
        else {
            next_event_time = first_departure_time;
            next_event_type = DEPARTURE;
        }

        // Update the master clock
        master_clock = next_event_time;

        // Handle the event
        if (next_event_type == ARRIVAL) { // If it is an arrival event
            auto new_job = Job(job_data[0]);
            job_data.pop_front(); // The first job is consumed 
            // Push into server if there is an empty server
            auto empty_server_id = server_controller.find_empty_server();
            if (empty_server_id != UNSIGNED_INF) {
                server_controller.put_job_to_server(new_job, empty_server_id, master_clock);
            } else {
                dispatcher.recv_job(new_job); // All servers are busy, push into dispatcher
            }
        } else { // If it is an depatrue event
            auto finished_job = server_controller.dep_job_from_server(first_departure_server);
            // Record an entry
            auto entry = std::tuple<double, double, unsigned, unsigned>{finished_job.get_arr_time(), finished_job.get_dep_time(), finished_job.get_c(), finished_job.total_job()};
            complete_entry.emplace_back(entry);

            if (finished_job.need_further_process()) { // Put the job into dispatcher if it need further process
                dispatcher.recv_job(finished_job);
            }
            else { // The job depart from the system permenently
                permenent_depature.emplace_back(finished_job);
            }

            // If there is jobs in dispatcher
            if (dispatcher.number_of_jobs() > 0) {
                server_controller.put_job_to_server(dispatcher.give_job(), first_departure_server, master_clock);
            }
        }
    }

    for (auto entry : complete_entry) {
        printMyEntry(entry);
    }

    return 0;
}
