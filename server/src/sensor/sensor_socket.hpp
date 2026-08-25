
#pragma once

#include <queue>

#include <boost/asio.hpp>

#include "../../data_bus.hpp"

using boost::asio::ip::tcp;

class SensorSocket {
    public:
        SensorSocket(tcp::socket socket, std::queue<DataBus>& req_bus, std::mutex& req_mutex)
            : socket_(std::move(socket)), req_bus_(req_bus), req_mutex_(req_mutex)  {
                boost::system::error_code ec;

                auto endpoint = socket_.remote_endpoint(ec);
                if (!ec) {
                    ip = endpoint.address().to_string();
                }
            }
        virtual ~SensorSocket() = default;
        virtual std::string parser() = 0;

    protected:
        tcp::socket socket_;
        std::string ip;
        std::queue<DataBus>& req_bus_;
        std::mutex& req_mutex_;
};
