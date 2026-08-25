
#pragma once

#include <algorithm>

#include "../sensor_socket.hpp"

class Movement : public SensorSocket {
    public:
        Movement(tcp::socket socket, std::queue<DataBus>& req_bus, std::mutex& req_mutex)
            : SensorSocket(std::move(socket), req_bus, req_mutex) {}

        std::string parser(const std::string& raw_data) override;
    private:
};
