#include <limits>
#include <deque>
#include <optional>
#include <vector>

#ifndef UTILITY_H
#define UTILITY_H

constexpr float FLOAT_INF = std::numeric_limits<float>::max();
constexpr unsigned UNSIGNED_INF = std::numeric_limits<unsigned>::max();

class Job {
    public:
        Job(double, std::deque<double>);
        unsigned get_c();
        bool need_further_process();
        double get_next_process_time();
        double get_dep_time();
        void set_dep_time(double);
        void update_c();
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
    std::deque<Job> jobs;
    
    friend class Dispatcher;
};

class Dispatcher {
    public:
        Dispatcher(unsigned);
        void recv_job(Job);
        Job give_job();
        size_t number_of_jobs();
    private:
        Queue high_priority_queue;
        Queue low_priority_queue;
        unsigned h;
};

class Server {
    Server(unsigned);
    void recv_job(Job, double);
    Job depart_job();
    double next_dep_time();

    unsigned id;
    std::optional<Job> job;
    bool is_busy;
    
    friend class ServerController;
};

class ServerController {
    public:
        ServerController(unsigned);
        unsigned find_empty_server();
        std::pair<double, unsigned> first_departure_time_server();
        bool server_busy();
        void put_job_to_server(Job, unsigned, double);
        Job  dep_job_from_server(unsigned);
    
    private:
        std::vector<Server> servers;
};

#endif