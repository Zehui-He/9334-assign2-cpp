#include <limits>
#include <deque>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <memory>

#pragma once

constexpr float FLOAT_INF = std::numeric_limits<float>::max();
constexpr unsigned UNSIGNED_INF = std::numeric_limits<unsigned>::max();

class Job {
    public:
        Job(std::pair<double, std::deque<double>>);
        unsigned get_c() const;
        bool need_further_process() const;
        double get_next_process_time();
        double get_dep_time() const;
        double get_arr_time() const;
        void set_dep_time(double);
        void update_c();
        unsigned total_job() const;
    private:
        std::deque<double> processing_times;
        unsigned c;
        double arrival_time;
        double depature_time;
        int job_num;
};


class Queue {
    enum struct Priority
    {
        high,
        low
    };
    Queue();
    Queue(Priority);
    Priority priority;
    std::deque<std::unique_ptr<Job>> jobs;
    
    friend class Dispatcher;
};

class Dispatcher {
    public:
        Dispatcher(unsigned);
        void recv_job(std::unique_ptr<Job>);
        std::unique_ptr<Job> give_job();
        size_t number_of_jobs() const;
    private:
        Queue high_priority_queue;
        Queue low_priority_queue;
        unsigned h;
};

class Server {
    Server(unsigned);
    void recv_job(std::unique_ptr<Job>, double);
    std::unique_ptr<Job> depart_job();
    double next_dep_time() const;

    unsigned id;
    std::unique_ptr<Job> job;
    bool is_busy;
    
    friend class ServerController;
};

class ServerController {
    public:
        ServerController(unsigned);
        unsigned find_empty_server() const;
        std::pair<double, unsigned> first_departure_time_server() const;
        bool server_busy() const;
        void put_job_to_server(std::unique_ptr<Job>, unsigned, double);
        std::unique_ptr<Job>  dep_job_from_server(unsigned);
    
    private:
        std::vector<Server> servers;
};

// class JobGenerator {
//     public:

//     private:
//         double lambda;
//         double alpha;
//         double beta;
//         double upper;
//         double lower;
//         double j_probs;

// };

class CannotReadFile : public std::exception {
    private: 
        std::string errorMessage;

    public:
        CannotReadFile(const std::string& filename) {
            std::stringstream ss;
            ss << "Cannot read the data file: " << filename << std::endl;
            errorMessage = ss.str();
        }

    const char* what() const noexcept override {
        return errorMessage.c_str();
    }
};

class NoJobInDispatcher : public std::exception {
    const char* what() const noexcept override {
        return "No job in the dispatcher";
    }
};

class NoTestIndex : public std::exception {
    const char* what() const noexcept override {
        return "No test index is given";
    }
};

std::deque<double> read_inter_arrival(std::string);
std::deque<std::deque<double>> read_processing_time(std::string);
std::deque<std::pair<double, std::deque<double>>> zip(std::deque<double>&&, std::deque<std::deque<double>>&&);
std::pair<unsigned, unsigned> read_para(std::string);
void printMyEntry(const std::tuple<double, double, unsigned, unsigned>&);