
#pragma once

#include <boost/asio.hpp>
#include <chrono>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using boost::asio::ip::tcp;
using boost::asio::ip::udp;

class PcWol {
        public:
            PcWol(boost::asio::io_context& io_context,  std::string ip, unsigned short port, std::string mac)
                : io_context_(io_context), ip_(ip), port_(port), mac_(mac) {}
            std::string executer(std::string commande);

        private:
            boost::asio::io_context& io_context_;
            std::string ip_;
            unsigned short port_;
            std::string mac_;

            bool est_allume();
            void envoyer_wake_on_lan();
};
