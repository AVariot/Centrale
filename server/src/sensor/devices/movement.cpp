
#include "movement.hpp"

std::string Movement::parser(const std::string& raw_data) {
    std::string cleaned_data = raw_data;
    cleaned_data.erase(std::remove(cleaned_data.begin(), cleaned_data.end(), '\n'), cleaned_data.end());
    cleaned_data.erase(std::remove(cleaned_data.begin(), cleaned_data.end(), '\r'), cleaned_data.end());

    std::cout << "MOVEMENT DATA: " << cleaned_data << std::endl;

    if (cleaned_data == "1") {
        return "ACTION:ARRIVEE";
    } 
    else if (cleaned_data == "-1") {
        return "ACTION:DEPART";
    }
    else {
        return "ACTION:INCONNU_OU_ERREUR";
    }
}
