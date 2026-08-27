
#include "main.hpp"

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
}

void Centrale::run() {

    std::thread thread_sensor([this]() {
        std::cout << "Launch sensor module" << std::endl;
        this->io_context_sensor.run();
    });

    while (1) {
    }
}

int main(void) {
    Centrale centrale;

    centrale.run();

    return 0;
}
