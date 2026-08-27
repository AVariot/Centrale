
#pragma once

#include <queue>
#include <memory>

#include <boost/asio.hpp>

#include "../../data_bus.hpp"

using boost::asio::ip::tcp;

class SensorSocket : public std::enable_shared_from_this<SensorSocket> {
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
        virtual std::string parser(const std::string& raw_data) = 0;

        void start_listening() {
            auto self(shared_from_this());

            std::cout << "[Network] En attente de lecture sur la socket pour " << this->ip << "..." << std::endl;

            socket_.async_read_some(boost::asio::buffer(buffer_, 1024),
                [this, self](boost::system::error_code ec, std::size_t bytes_recus) {
                    if (!ec) {
                        std::string true_data(buffer_, bytes_recus);
                        std::cout << "[Network][SENSOR] Got from " << this->ip << " : " << true_data << std::endl;

                        std::cout << "GOT: " << true_data << std::endl;
                        std::string cleaned_data = this->parser(true_data);
                        {
                            std::lock_guard<std::mutex> lock(this->req_mutex_);
                            this->req_bus_.push({"SENSOR", this->ip, cleaned_data});
                        }

                        this->start_listening();
                    } else {
                        std::cout << "[NETWORK][SENSOR] Deconnexion of " << this->ip << std::endl;
                    }
                });
        }

    protected:
        tcp::socket socket_;
        std::string ip;
        std::queue<DataBus>& req_bus_;
        std::mutex& req_mutex_;
        char buffer_[1024];
};
