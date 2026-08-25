
#pragma once

#include <unordered_map>
#include <queue>

#include <boost/asio.hpp>

#include "./sensor_socket.hpp"
#include "./devices/movement.hpp"

using boost::asio::ip::tcp;

class SensorModule {
    public:
        SensorModule(boost::asio::io_context &io_context, short port, std::queue<DataBus>& req_bus, std::mutex& req_mutex)
            : acceptor_(io_context, tcp::endpoint(tcp::v4(), port)), req_bus_(req_bus), req_mutex_(req_mutex) {
                this->waiting_client();
            }

    private:
        tcp::acceptor acceptor_;
        std::unordered_map<int, std::shared_ptr<SensorSocket>> sensor;
        std::queue<DataBus>& req_bus_;
        std::mutex& req_mutex_;

        void waiting_client() {
            acceptor_.async_accept([this](boost::system::error_code ec, tcp::socket socket) {
                if (!ec) {
                    std::shared_ptr<SensorSocket> new_sensor = std::make_shared<Movement>(std::move(socket), req_bus_, req_mutex_);
                    sensor.insert({sensor.size(), new_sensor} );
                    this->waiting_client();
                }
            });
        }
};
