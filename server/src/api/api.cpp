
#include "api.hpp"

Api::Api(net::io_context& ioc, short port, std::queue<DataBus>& req_bus, std::mutex& req_mutex, std::queue<DataBus>& res_bus, std::mutex& res_mutex)
    : ioc_(ioc), 
    req_bus_(req_bus),
    req_mutex_(req_mutex),
    res_bus_(res_bus),
    res_mutex_(res_mutex),
      acceptor_(ioc, tcp::endpoint(tcp::v4(), port)),
      router(std::make_unique<ApiRouter>())
{
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

    router->add_route(http::verb::post, "/api/status", [this](const auto& req) {
        http::response<http::string_body> res{http::status::ok, req.version()};
        res.set(http::field::content_type, "application/json");
        std::string payload = req.body();
        std::string response;
        {
            std::lock_guard<std::mutex> lock(this->req_mutex_);
            this->req_bus_.push({"API", "mobile", payload});
        }
        int retries = 0;
        while (retries < 20) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            
            std::lock_guard<std::mutex> lock(this->res_mutex_); 
            if (!this->res_bus_.empty()) {
                response = this->res_bus_.front().body; 
                this->res_bus_.pop(); 
                break;
            }
            retries++;
        }
        res.body() = "{\"response\": \"" + response + "\"}";
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
