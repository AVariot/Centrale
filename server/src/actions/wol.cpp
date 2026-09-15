
#include "./wol.hpp"

std::string PcWol::executer(std::string commande) {
    if (commande != "wake") {
        return "ko";
    }

    if (est_allume()) {
        std::cout << "[PcWol] Already wake up" << std::endl;
        return "ok";
    }

    envoyer_wake_on_lan();
    return "ok";
}

bool PcWol::est_allume() {
    boost::system::error_code ec_parse;
    auto address = boost::asio::ip::make_address(ip_, ec_parse);
    if (ec_parse) {
        return false;
    }
    tcp::endpoint endpoint(address, port_);

    boost::asio::io_context probe_context;
    tcp::socket socket(probe_context);
    boost::asio::steady_timer timer(probe_context);
    bool connected = false;

    timer.expires_after(std::chrono::milliseconds(500));
    timer.async_wait([&socket](const boost::system::error_code& ec) {
        if (!ec) {
            boost::system::error_code ignore;
            socket.cancel(ignore);
        }
    });

    socket.async_connect(endpoint, [&](const boost::system::error_code& ec) {
        connected = !ec;
        timer.cancel();
    });

    probe_context.run();

    return connected;
}

void PcWol::envoyer_wake_on_lan() {
    try {
        // MAC en octets
        std::vector<uint8_t> mac_bytes;
        unsigned int byte;
        char colon;
        std::stringstream ss(mac_);
        for (int i = 0; i < 6; ++i) {
            ss >> std::hex >> byte;
            mac_bytes.push_back(static_cast<uint8_t>(byte));
            if (i < 5) ss >> colon;
        }

        // Magic packet (6 octets FF, puis 16 fois la MAC)
        std::vector<uint8_t> magic_packet(102);
        for (int i = 0; i < 6; ++i) magic_packet[i] = 0xFF;
        for (int i = 1; i <= 16; ++i) {
            for (int j = 0; j < 6; ++j) {
                magic_packet[i * 6 + j] = mac_bytes[j];
            }
        }

        // UDP in boradcats port 9
        udp::socket socket(io_context_, udp::v4());
        socket.set_option(boost::asio::socket_base::broadcast(true));

        udp::endpoint broadcast_endpoint(boost::asio::ip::address_v4::broadcast(), 9);
        socket.send_to(boost::asio::buffer(magic_packet), broadcast_endpoint);

        std::cout << "[PcWol] Packet sent to : " << mac_ << std::endl;

    } catch (std::exception& e) {
        std::cerr << "[PcWol] Erreur WoL : " << e.what() << std::endl;
    }
}
