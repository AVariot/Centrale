
#pragma once

#include <list>
#include <fstream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <atomic>

#include "../../data_bus.hpp"

class Log {
    public:
        Log();
        ~Log();
        void insert_log(DataBus data_to_log);
        std::string get_log();

    private:
        void write_loop();

        std::list<DataBus> log_memory_trace;
        std::string path_to_log;
        bool log_saved;

        std::queue<DataBus> write_queue;
        std::mutex write_mutex;
        std::condition_variable write_cv;
        std::atomic<bool> stop_thread;
        std::thread write_thread;
        std::string data_bus_stringify(DataBus dtbs);
};
