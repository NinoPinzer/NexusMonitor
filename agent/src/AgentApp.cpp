#include "AgentApp.hpp"
#include <iostream>
#include <iomanip>  // Für Formatierung der Ausgabe (std::fixed, std::setprecision, std::put_time)
#include <chrono>   // Für std::chrono::system_clock
#include <ctime>    // Für std::time_t und std::localtime

// --- CPU Stats ---
void printCpuStats(const CpuStats& stats) {
    std::cout << "--- CPU Stats ---" << std::endl;
    std::time_t timestamp_t = std::chrono::system_clock::to_time_t(stats.timestamp);
    std::tm* local_tm = std::localtime(&timestamp_t); // Konvertiert zu lokaler Zeit

    std::cout << "Timestamp: " << std::put_time(local_tm, "%Y-%m-%d %H:%M:%S") << std::endl;
    std::cout << "Usage:     " << std::fixed << std::setprecision(2) << stats.usagePercent << "%" << std::endl;
    std::cout << "Idle:      " << std::fixed << std::setprecision(2) << stats.idlePercent << "%" << std::endl;
    std::cout << "User:      " << std::fixed << std::setprecision(2) << stats.userPercent << "%" << std::endl;
    std::cout << "System:    " << std::fixed << std::setprecision(2) << stats.systemPercent << "%" << std::endl;
    std::cout << "-----------------" << std::endl << std::endl;
}

// --- Memory Stats ---
void printMemoryStats(const MemoryStats& stats) {
    std::cout << "--- Memory Stats ---" << std::endl;
    std::time_t timestamp_t = std::chrono::system_clock::to_time_t(stats.timestamp);
    std::tm* local_tm = std::localtime(&timestamp_t); // Konvertiert zu lokaler Zeit

    std::cout << "Timestamp:   " << std::put_time(local_tm, "%Y-%m-%d %H:%M:%S") << std::endl;
    std::cout << "Total:       " << stats.totalBytes / (1024.0 * 1024.0 * 1024.0) << " GB" << std::endl;
    std::cout << "Used:        " << stats.usedBytes / (1024.0 * 1024.0 * 1024.0) << " GB" << std::endl;
    std::cout << "Free:        " << stats.freeBytes / (1024.0 * 1024.0 * 1024.0) << " GB" << std::endl;
    std::cout << "Usage:       " << std::fixed << std::setprecision(2) << stats.usagePercent << "%" << std::endl;
    std::cout << "Swap Total:  " << stats.swapTotalBytes / (1024.0 * 1024.0 * 1024.0) << " GB" << std::endl;
    std::cout << "Swap Used:   " << stats.swapUsedBytes / (1024.0 * 1024.0 * 1024.0) << " GB" << std::endl;
    std::cout << "--------------------" << std::endl << std::endl;
}

// --- Network I/O Stats ---
void printNetworkIoStats(const std::vector<NetworkIoStats>& stats_list) {
    std::cout << "--- Network I/O Stats ---" << std::endl;
    if (stats_list.empty()) {
        std::cout << "No network interfaces found or data available." << std::endl;
        return;
    }

    // Für NetworkIoStats nehmen wir den Timestamp des ersten Eintrags, da alle gleich sein sollten.
    std::time_t timestamp_t = std::chrono::system_clock::to_time_t(stats_list[0].timestamp);
    std::tm* local_tm = std::localtime(&timestamp_t); // Konvertiert zu lokaler Zeit

    std::cout << "Timestamp: " << std::put_time(local_tm, "%Y-%m-%d %H:%M:%S") << std::endl;

    for (const auto& stats : stats_list) {
        std::cout << "  Interface:   " << stats.interfaceName << std::endl;
        std::cout << "    Bytes Sent:    " << stats.bytesSent << std::endl;
        std::cout << "    Bytes Received: " << stats.bytesReceived << std::endl;
        std::cout << "    Packets Sent:   " << stats.packetsSent << std::endl;
        std::cout << "    Packets Received: " << stats.packetsReceived << std::endl;
        std::cout << "  -----------------------" << std::endl;
    }
    std::cout << "-------------------------" << std::endl << std::endl;
}

// --- AgentApp Methoden ---
AgentApp::AgentApp() {
    std::cout << "AgentApp initialized." << std::endl;
}

void AgentApp::run() {
    std::cout << "AgentApp started. Collecting metrics every 5 seconds..." << std::endl;
    while (true) {
        collectAndSendMetrics();
        std::this_thread::sleep_for(std::chrono::seconds(5)); // Alle 5 Sekunden Metriken sammeln
    }
}

void AgentApp::collectAndSendMetrics() {
    std::cout << "Collecting metrics..." << std::endl;
    try {
        CpuStats cpu = sysMonitor.getCpuStats();
        printCpuStats(cpu); // Nur zum Debuggen/Anzeigen

        MemoryStats mem = sysMonitor.getMemoryStats();
        printMemoryStats(mem);

        std::vector<NetworkIoStats> net = sysMonitor.getNetworkIoStats();
        printNetworkIoStats(net);


    } catch (const std::exception& e) {
        std::cerr << "Error collecting metrics: " << e.what() << std::endl;
    }
}