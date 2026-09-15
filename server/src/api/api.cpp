
#include "api.hpp"

Api::Api(net::io_context& ioc, short port, std::queue<DataBus>& req_bus, std::mutex& req_mutex)
    : ioc_(ioc), 
    req_bus_(req_bus),
    req_mutex_(req_mutex),
      acceptor_(ioc, tcp::endpoint(tcp::v4(), port)),
      router(std::make_unique<ApiRouter>())
{
    // On enregistre les routes dès le démarrage
    std::cout << "Start At: " << port << std::endl;
    init_router();
}

void Api::init_router() {
    router->add_route(http::verb::get, "/api/status", [](const auto& req) {
        http::response<http::string_body> res{http::status::ok, req.version()};
        res.set(http::field::content_type, "application/json");
        res.body() = R"({"status": "API en ligne", "version": "1.0"})";
        res.prepare_payload();
        return res;
    });
}

void Api::run() {
    std::cout << "Serveur demarre sur le port " << acceptor_.local_endpoint().port() << std::endl;

    while (true) {
        tcp::socket socket(ioc_);
        
        acceptor_.accept(socket);
        
        std::thread(&Api::do_session, this, std::move(socket)).detach();
    }
}

// HTTP Cycle
void Api::do_session(tcp::socket socket) {
    boost::system::error_code ec;
    boost::beast::flat_buffer buffer;

    for(;;) {
        http::request<http::string_body> req;
        
        http::read(socket, buffer, req, ec);
        
        if (ec == http::error::end_of_stream) break;
        if (ec) { std::cerr << "Erreur de lecture: " << ec.message() << "\n"; break; }

        auto res = router->route_request(req);

        http::write(socket, res, ec);
        if (ec) { std::cerr << "Erreur d'ecriture: " << ec.message() << "\n"; break; }

        if (!res.keep_alive()) break;
    }
    
    socket.shutdown(tcp::socket::shutdown_send, ec);
}
