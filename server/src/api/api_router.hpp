#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <unordered_map>
#include <functional>
#include <string>
#include <iostream>

namespace http = boost::beast::http;

class ApiRouter {
public:
    // Définition des types pour simplifier la lecture
    using Request = http::request<http::string_body>;
    using Response = http::response<http::string_body>;
    using RouteHandler = std::function<Response(const Request&)>;

    // Ajoute une nouvelle route
    void add_route(http::verb method, const std::string& target, RouteHandler handler) {
        std::string route_key = std::string(http::to_string(method)) + ":" + target;
        routes[route_key] = handler;
    }

    // Traite la requête entrante
    Response route_request(const Request& req) {
        std::string route_key = std::string(http::to_string(req.method())) + ":" + std::string(req.target());

        auto it = routes.find(route_key);
        if (it != routes.end()) {
            // Exécute le callback si la route est trouvée
            return it->second(req);
        }

        // Réponse 404 par défaut si la route n'existe pas
        Response res{http::status::not_found, req.version()};
        res.set(http::field::server, "BeastApiServer");
        res.set(http::field::content_type, "application/json");
        res.body() = R"({"erreur": "Route introuvable"})";
        res.prepare_payload();
        return res;
    }

private:
    std::unordered_map<std::string, RouteHandler> routes;
};
