
#pragma once

#include <thread>
#include <mutex>
#include <queue>

#include <boost/asio.hpp>

#include "./src/log/log.hpp"
#include "./src/api/api.hpp"
#include "./src/sensor/sensor_module.hpp"
#include "./data_bus.hpp"
#include "./src/actions/wol.hpp"

/*
    INPUT: [FUNCTION HANDLER HERE]
        Sensor movement: IN | OUT
        Sensor lumineu: ON | OFF

    OUTPUT: []
        wol
        luniere

    ASK: [API]
        getLumiere
*/

class Centrale {
    public:
    Centrale();
    void run();
    using Command = std::function<void(std::string, std::string)>;

    private:
        std::queue<DataBus> req_bus;
        std::mutex req_bus_mutex;
        std::queue<DataBus> res_bus;
        std::mutex res_bus_mutex;
        std::unique_ptr<Log> log_ptr;
        std::unique_ptr<Api> api_ptr;
        std::unique_ptr<SensorModule> sensor_ptr;
        std::unique_ptr<PcWol> wol;
        boost::asio::io_context io_context_sensor;
        boost::asio::io_context io_context_api;
        std::unordered_map<std::string, Command> command;

        void move_command(std::string input, std::string request_id);
        void lux_command(std::string input, std::string request_id);
        void log_command(std::string input, std::string request_id);
};
