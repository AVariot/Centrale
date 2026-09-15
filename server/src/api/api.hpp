
#pragma once

#include "./api_router.hpp"
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <memory>
#include <thread>
#include <queue>
#include <data_bus.hpp>

namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;
namespace http = boost::beast::http;

class Api {
    public:
        Api(net::io_context& ioc, short port, std::queue<DataBus>& req_bus, std::mutex& req_mutex);
        void run();

    private:
        void do_session(tcp::socket socket);
        void init_router();
        std::unique_ptr<ApiRouter> router;
        net::io_context& ioc_;
        tcp::acceptor acceptor_;
        std::queue<DataBus>& req_bus_;
        std::mutex& req_mutex_;
};
