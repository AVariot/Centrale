
#pragma once

#include "../sensor_socket.hpp"

class Movement : public SensorSocket {
    public:
        Movement(tcp::socket socket, std::queue<DataBus>& req_bus, std::mutex& req_mutex)
            : SensorSocket(std::move(socket), req_bus, req_mutex) {}

        std::string parser() override;
    private:
};
