
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


class Centrale {
    public:
        Centrale();
        void run();

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
};
