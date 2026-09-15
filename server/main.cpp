
#include "main.hpp"

#include <cstdlib>

//FIXME: iomplement api ina thread with data bus req et res

namespace {
    std::string env_or(const char* name, const std::string& default_value) {
        const char* value = std::getenv(name);
        return value ? std::string(value) : default_value;
    }
}

/*
    Main Thread = Analyse and Orchestre
    Thread 1 = External Connexion = API exposed
    Thread 2 = Socket for ESP32
    Thread 3 = Logging
*/
/*
    Double bus for action
    Shared for state or informations module
*/

Centrale::Centrale() {
    std::cout << "Centrale initialing" << std::endl;
    log_ptr = std::make_unique<Log>();
    api_ptr = std::make_unique<Api>(io_context_api, 9001, req_bus, req_bus_mutex, res_bus, res_bus_mutex);
    sensor_ptr = std::make_unique<SensorModule>(io_context_sensor, 9000, req_bus, req_bus_mutex);

    std::string pc_ip = env_or("PC_IP", "192.168.1.100");
    unsigned short pc_port = static_cast<unsigned short>(std::stoi(env_or("PC_PORT", "22")));
    std::string pc_mac = env_or("PC_MAC", "00:00:00:00:00:00");
    wol = std::make_unique<PcWol>(io_context_sensor, pc_ip, pc_port, pc_mac);
    command["MOVE"] = [this](std::string input, std::string request_id) {
        this->move_command(input, request_id);
    };
    command["LUX"] = [this](std::string input, std::string request_id) {
        this->lux_command(input, request_id);
    };
}

void Centrale::move_command(std::string input, std::string request_id) {
    auto pos = input.find("MOVE");

    if (pos == std::string::npos) {
        return;
    }

    if (input.substr(pos + 1, input.length()) == "IN") {
    } else if (input.substr(pos + 1, input.length()) == "OUT") {
        log_ptr->insert_log(DataBus{request_id, "Error:", "Command not found: " + input.substr(pos + 1, input.length()) });
    }
}

void Centrale::lux_command(std::string input, std::string request_id) {
}

void Centrale::run() {

    std::thread thread_sensor([this]() {
        std::cout << "Launch sensor module" << std::endl;
        this->io_context_sensor.run();
    });

    thread_sensor.detach();

    std::thread thread_api([this]() {
        std::cout << "Launch api module" << std::endl;
        this->io_context_api.run();
        this->api_ptr->run();
    });

    thread_api.detach();

    while (1) {
        DataBus request;
        bool has_request = false;
        {
            std::lock_guard<std::mutex> lock(req_bus_mutex);
            if (!req_bus.empty()) {
                request = req_bus.front();
                req_bus.pop();
                has_request = true;
            }
        }

        if (has_request) {
            std::cout << "[Centrale] Requete recue: " << request.id << " / " << request.target << " / " << request.body << std::endl;
            log_ptr->insert_log(request);

            bool command_found = false;

            for (const auto& [key, func] : command) {
                if (request.body.find(key) != std::string::npos) {
                    log_ptr->insert_log(DataBus{request.id, "Success:", "Command found: " + request.body});
                    func(request.body, request.id);
                    command_found = true;
                    break;
                }
            }

            if (!command_found) {
                {
                    std::lock_guard<std::mutex> lock(res_bus_mutex);
                    this->res_bus.push({"API", "mobile", "Error: command not found"});
                }
                log_ptr->insert_log(DataBus{request.id, "mobile", "Error: Command not found: " + request.body});
            }

            // if (request.body == "ACTION:IN") {
            //     std::string result = wol->executer("wake");
            //     std::cout << "[Centrale] WOL result: " << result << std::endl;
            //     log_ptr->insert_log(DataBus{request.id, "wol", result});
            // }
        }
    }
}

int main(void) {
    Centrale centrale;

    centrale.run();

    return 0;
}
