#include "../include/utility.h"
#include <deque>
#include <string>
#include <cstring>
#include <iostream>
#include <sstream>

// A job should be initilized with arrival time and 
// a set of processing time. 
Job::Job(std::pair<double, std::deque<double>> arrival_procs) {
    auto [arrival_time, processing_times] = arrival_procs;
    this->processing_times = processing_times;
    this->c = 0;
    this->arrival_time = arrival_time;
    this->depature_time = FLOAT_INF;
    this->job_num = processing_times.size();
};

// Get the number of completion of the job. 
auto Job::get_c() const -> unsigned {
    return this->c;
}

// Return true if the job requires further process. 
auto Job::need_further_process() const -> bool {
    return this->processing_times.size() > 0;
}

// Return the next processing time of the job.
// This would remove the processing time from the job.
auto Job::get_next_process_time() -> double {
    auto processing_time = this->processing_times.front();
    this->processing_times.pop_front();
    return processing_time;
}

//
auto Job::get_dep_time() const -> double {
    return this->depature_time;
}

auto Job::get_arr_time() const -> double {
    return this->arrival_time;
}

auto Job::total_job() const -> unsigned {
    return this->job_num;
}

auto Job::set_dep_time(double dep_time) -> void {
    this->depature_time = dep_time;
}

auto Job::update_c() -> void {
    this->c += 1;
}

// Initilize a queue with given priority. Only the 
// dispatcher can instantize a queuqe. 
Queue::Queue(Priority priority) {
    this->priority = priority;
    this->jobs = std::deque<std::unique_ptr<Job>>{};
}

// Initilize a queue without given priority. The queue
// is high priority by default.
Queue::Queue() {
    this->priority = Priority::high;
    this->jobs = std::deque<std::unique_ptr<Job>>{};
}


Dispatcher::Dispatcher(unsigned h) {
    this->h = h;
    this->high_priority_queue = Queue(Queue::Priority::high);
    this->low_priority_queue = Queue(Queue::Priority::low);
}

// Put a job into the dispatcher. If the c of job is greater than 
// h, the job is placed into the low priority queue. Otherwise, it 
// is placed in the high priority queue. 
auto Dispatcher::recv_job(std::unique_ptr<Job> job) -> void {
    if (job->get_c() >= this->h) {
        this->low_priority_queue.jobs.push_back(std::move(job));
        return;
    }
    this->high_priority_queue.jobs.push_back(std::move(job));
}


auto Dispatcher::give_job() -> std::unique_ptr<Job> {
    if (this->high_priority_queue.jobs.size() > 0) {
        auto res = std::move(this->high_priority_queue.jobs.front());
        this->high_priority_queue.jobs.pop_front();
        return res;
    }
    else if (this->low_priority_queue.jobs.size() > 0) {
        auto res = std::move(this->low_priority_queue.jobs.front());
        this->low_priority_queue.jobs.pop_front();
        return res;
    }
    else {
        throw NoJobInDispatcher();
    }
}

// Give the total number of jobs that is in the dispacther. 
auto Dispatcher::number_of_jobs() const -> size_t {
    return this->high_priority_queue.jobs.size() + this->low_priority_queue.jobs.size();
}


Server::Server(unsigned id) {
    this->is_busy = false;
    this->id = id;
    this->job = nullptr;
}

auto Server::recv_job(std::unique_ptr<Job> job, double master_clock) -> void {
    auto processe_time = job->get_next_process_time();
    job->set_dep_time(master_clock + processe_time);
    this->is_busy = true;
    this->job = std::move(job);
}


auto Server::depart_job() -> std::unique_ptr<Job> {
    auto completed_job = std::move(this->job);
    completed_job->update_c();
    this->job = nullptr;
    this->is_busy = false;
    return completed_job;
}

auto Server::next_dep_time() const -> double {
    if (!this->is_busy) {
        return FLOAT_INF;
    }
    return this->job->get_dep_time();
}

ServerController::ServerController(unsigned num_server) {
    for (auto i = 0; i < num_server; i++) {
        this->servers.push_back(Server(i));
    }
}

auto ServerController::find_empty_server() const -> unsigned {
    for (auto server = this->servers.begin(); server != this->servers.end(); server++) {
        if (!server->is_busy) {
            return server->id;
        }
    }
    return UNSIGNED_INF;
}

auto ServerController::first_departure_time_server() const -> std::pair<double, unsigned> {
    auto server_idx = UNSIGNED_INF;
    auto dep_time = FLOAT_INF;

    for (auto i = 0; i < this->servers.size(); i++) {
        if (this->servers[i].is_busy) {
            if (this->servers[i].job->get_dep_time() < dep_time) {
                dep_time = servers[i].job->get_dep_time();
                server_idx = i;
            }
        }
    }
    return std::pair<double, unsigned>{dep_time, server_idx};
}

auto ServerController::server_busy() const -> bool {
    for (auto server = this->servers.begin(); server != this->servers.end(); server++) {
        if (server->is_busy) {
            return true;
        }
    }
    return false;
}

auto ServerController::put_job_to_server(std::unique_ptr<Job> job, unsigned server_id, double master_clock) -> void {
    this->servers[server_id].recv_job(std::move(job), master_clock);
}

auto ServerController::dep_job_from_server(unsigned server_id) -> std::unique_ptr<Job> {
    return this->servers[server_id].depart_job();
}

auto read_inter_arrival(std::string filename) -> std::deque<double> {
    auto file = std::ifstream(filename);
    auto res = std::deque<double>{};

    if (!file.is_open()) {
        throw CannotReadFile(filename);
    }

    double arrival_time;
    while (file >> arrival_time) {
        res.push_back(arrival_time);
    }

    // 
    for (auto i = 1; i < res.size(); i++) {
        res[i] = res[i - 1] + res[i];
    }

    file.close();
    return res;
}

auto read_processing_time(std::string filename) -> std::deque<std::deque<double>> {
    auto file = std::ifstream(filename);
    auto res = std::deque<std::deque<double>>{};

    if (!file.is_open()) {
        throw CannotReadFile(filename);
    }

    std::string line;
    while (std::getline(file, line)) {
        auto process_time = std::deque<double>{};

        std::stringstream item_stream(line);
        std::string procs_time;
        while (std::getline(item_stream, procs_time, ' ')) { // Read the processing time into the buffer 
            if (strcmp(procs_time.c_str(), "NaN") == 0) { // Stop if the processing time equals to "NaN"
                break;
            }
            process_time.push_back(std::stod(procs_time.c_str()));
        }
        res.push_back(process_time);
    }
    
    file.close();
    return res;
}

auto zip(std::deque<double>&& a, std::deque<std::deque<double>>&& b) -> std::deque<std::pair<double, std::deque<double>>> {
    auto res = std::deque<std::pair<double, std::deque<double>>>{};
    for (auto i = 0; i < a.size(); i++) {
        res.emplace_back(a[i], b[i]);
    }
    return res;
}

auto read_para(std::string filename) -> std::pair<unsigned,unsigned> {
    auto file = std::ifstream(filename);

    if (!file.is_open()) {
        throw CannotReadFile(filename);
    }

    unsigned threshold;
    unsigned num_server;
    file >> threshold;
    file >> num_server;

    file.close();
    return std::pair<unsigned, unsigned>{threshold, num_server};
}

auto printMyEntry(const std::tuple<double, double, unsigned, unsigned>& entry) -> void {
    std::cout << std::setprecision(4) << std::fixed << std::get<0>(entry) << " " << std::get<1>(entry) << " " << std::get<2>(entry) << " " << std::get<3>(entry) << " " << std::endl;
}
