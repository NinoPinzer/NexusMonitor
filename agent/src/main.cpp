#include "AgentApp.hpp"
#include <iostream>

int main() {
    std::cout << "Starting Agent Application..." << std::endl;
    AgentApp app;
    app.run(); // Startet den Hauptloop des Agenten
    std::cout << "Agent Application stopped." << std::endl;
    return 0;
}