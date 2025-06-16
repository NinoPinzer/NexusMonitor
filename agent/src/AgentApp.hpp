#pragma once

#include <string>
#include <chrono>
#include <thread>
#include <vector>

// Inkludiere die SysMonitor-Header
#include "SysMonitor.hpp"
#include "DataTypes.hpp" // Für die Metrik-Strukturen


class AgentApp {
public:
    AgentApp();
    void run();

private:
    SysMonitor sysMonitor;


    void collectAndSendMetrics();
};