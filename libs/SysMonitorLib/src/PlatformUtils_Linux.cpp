#include "PlatformUtils.hpp"
#include <fstream>
#include <sstream>
#include <string>
#include <iostream> // Nur zum Debuggen

#if defined(__linux__)

CpuStats get_cpu_stats_linux() {
    CpuStats stats;
    std::ifstream proc_stat("/proc/stat");
    if (!proc_stat.is_open()) {
        return stats;
    }

    std::string line;
    std::getline(proc_stat, line);
    std::stringstream ss(line);
    std::string cpu_label;
    long long user, nice, system, idle, iowait, irq, softirq, steal, guest, guest_nice;

    ss >> cpu_label >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal >> guest >> guest_nice;

    stats.userCycles = user + nice;
    stats.systemCycles = system;
    stats.idleCycles = idle;
    stats.totalCycles = stats.userCycles + stats.systemCycles + stats.idleCycles + iowait + irq + softirq + steal; // Vereinfacht

    // Für prozentuale Nutzung bräuchte man Delta-Werte von einem vorherigen Aufruf
    stats.usagePercent = 0.0; // Muss später berechnet werden
    stats.idlePercent = 0.0;  // Muss später berechnet werden
    stats.timestamp = std::chrono::system_clock::now();

    return stats;
}

MemoryStats get_memory_stats_linux() {
    MemoryStats stats;
    std::ifstream proc_meminfo("/proc/meminfo");
    if (!proc_meminfo.is_open()) {
        return stats;
    }

    std::string line;
    while (std::getline(proc_meminfo, line)) {
        if (line.rfind("MemTotal:", 0) == 0) {
            stats.totalBytes = std::stoll(line.substr(10)) * 1024; // Convert KB to Bytes
        } else if (line.rfind("MemAvailable:", 0) == 0) {
            stats.freeBytes = std::stoll(line.substr(13)) * 1024;
        } else if (line.rfind("SwapTotal:", 0) == 0) {
            stats.swapTotalBytes = std::stoll(line.substr(10)) * 1024;
        } else if (line.rfind("SwapFree:", 0) == 0) {
            stats.swapFreeBytes = std::stoll(line.substr(9)) * 1024;
        }
    }
    stats.usedBytes = stats.totalBytes - stats.freeBytes;
    if (stats.totalBytes > 0) {
        stats.usagePercent = static_cast<double>(stats.usedBytes) / stats.totalBytes * 100.0;
    }
    stats.swapUsedBytes = stats.swapTotalBytes - stats.swapFreeBytes;
    stats.timestamp = std::chrono::system_clock::now();
    return stats;
}

#endif // __linux__