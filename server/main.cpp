
#include "main.hpp"

#include <cstdlib>

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
    api_ptr = std::make_unique<Api>();
    sensor_ptr = std::make_unique<SensorModule>(io_context_sensor, 9000, req_bus, req_bus_mutex);

    std::string pc_ip = env_or("PC_IP", "192.168.1.100");
    unsigned short pc_port = static_cast<unsigned short>(std::stoi(env_or("PC_PORT", "22")));
    std::string pc_mac = env_or("PC_MAC", "00:00:00:00:00:00");
    wol = std::make_unique<PcWol>(io_context_sensor, pc_ip, pc_port, pc_mac);
}

void Centrale::run() {

    std::thread thread_sensor([this]() {
        std::cout << "Launch sensor module" << std::endl;
        this->io_context_sensor.run();
    });

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

            if (request.body == "ACTION:ARRIVEE") {
                std::string result = wol->executer("wake");
                std::cout << "[Centrale] WOL result: " << result << std::endl;
                log_ptr->insert_log(DataBus{request.id, "wol", result});
            }
        }
    }
}

int main(void) {
    Centrale centrale;

    centrale.run();

    return 0;
}
