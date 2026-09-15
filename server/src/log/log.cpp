
#include "log.hpp"

#include <filesystem>

Log::Log() : log_saved(false), stop_thread(false) {
    std::string buffer;
    std::ifstream config_file("./src/config/log.cfg");

    if (!config_file.is_open()) {
        return;
    }

    while (getline (config_file, buffer)) {
        auto index = buffer.find("=");
        if (index == std::string::npos) {
            std::cout << "Error in log config file: " << buffer << std::endl;
            continue;
        }

        std::string key = buffer.substr(0, index);
        std::string value = buffer.substr(index + 1);

        if (key == "dest") {
            path_to_log = value;
        } else if (key == "save_trace") {
            log_saved = (value == "true");
        } else {
            std::cout << "Load config file error: Unknwokn key " << key << std::endl;
        }
    }

    config_file.close();

    if (log_saved) {
        std::filesystem::create_directories(path_to_log);
        write_thread = std::thread(&Log::write_loop, this);
    }
}

Log::~Log() {
    if (log_saved) {
        {
            std::lock_guard<std::mutex> lock(write_mutex);
            stop_thread = true;
        }
        write_cv.notify_all();
        if (write_thread.joinable()) {
            write_thread.join();
        }
    }
}

void Log::insert_log(DataBus data_to_log) {
    log_memory_trace.push_front(data_to_log);

    if (log_saved) {
        {
            std::lock_guard<std::mutex> lock(write_mutex);
            write_queue.push(data_to_log);
        }
        write_cv.notify_one();
    }
}

void Log::write_loop() {
    std::ofstream log_file(path_to_log + "centrale.log", std::ios::app);

    if (!log_file.is_open()) {
        std::cout << "Error: unable to open log file at " << path_to_log << std::endl;
        return;
    }

    while (true) {
        std::unique_lock<std::mutex> lock(write_mutex);
        write_cv.wait(lock, [this]() { return stop_thread || !write_queue.empty(); });

        while (!write_queue.empty()) {
            DataBus entry = write_queue.front();
            write_queue.pop();
            lock.unlock();

            log_file << "[" << entry.id << "]" << "[" << entry.target << "]: " << entry.body << std::endl;

            lock.lock();
        }

        if (stop_thread) {
            break;
        }
    }

    log_file.close();
}
