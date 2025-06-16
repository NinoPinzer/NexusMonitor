#include "SysMonitor.hpp"
#include "PlatformUtils.hpp" // Die Schnittstelle zu den plattformspezifischen Helfern
#include <stdexcept>

SysMonitor::SysMonitor() {
    // Initialisierung, falls nötig
}

SysMonitor::~SysMonitor() {
    // Aufräumen, falls nötig
}

CpuStats SysMonitor::getCpuStats() const {
    // Hier wird die plattformspezifische Funktion aufgerufen
#if defined(_WIN32)
    return get_cpu_stats_win();
#elif defined(__linux__)
    return get_cpu_stats_linux();
#elif defined(__APPLE__)
    return get_cpu_stats_mac();
#else
    throw std::runtime_error("Unsupported platform for CPU stats.");
#endif
}

MemoryStats SysMonitor::getMemoryStats() const {
#if defined(_WIN32)
    return get_memory_stats_win();
#elif defined(__linux__)
    return get_memory_stats_linux();
#elif defined(__APPLE__)
    return get_memory_stats_mac();
#else
    throw std::runtime_error("Unsupported platform for Memory stats.");
#endif
}

std::vector<NetworkIoStats> SysMonitor::getNetworkIoStats() const {
#if defined(_WIN32)
    return get_network_io_stats_win();
#elif defined(__linux__)
    return get_network_io_stats_linux();
#elif defined(__APPLE__)
    return get_network_io_stats_mac();
#else
    throw std::runtime_error("Unsupported platform for Network I/O stats.");
#endif
}

DiskIoStats SysMonitor::getDiskIoStats(const std::string& diskName) const {
#if defined(_WIN32)
    return get_disk_io_stats_win(diskName);
#elif defined(__linux__)
    return get_disk_io_stats_linux(diskName);
#elif defined(__APPLE__)
    return get_disk_io_stats_mac(diskName);
#else
    throw std::runtime_error("Unsupported platform for Disk I/O stats.");
#endif
}

std::vector<ProcessInfo> SysMonitor::getProcessList() const {
#if defined(_WIN32)
    return get_process_list_win();
#elif defined(__linux__)
    return get_process_list_linux();
#elif defined(__APPLE__)
    return get_process_list_mac();
#else
    throw std::runtime_error("Unsupported platform for Process list.");
#endif
}

ServiceStatus SysMonitor::getServiceStatus(const std::string& serviceName) const {
#if defined(_WIN32)
    return get_service_status_win(serviceName);
#elif defined(__linux__)
    return get_service_status_linux(serviceName);
#elif defined(__APPLE__)
    return get_service_status_mac(serviceName);
#else
    throw std::runtime_error("Unsupported platform for Service status.");
#endif
}

std::vector<LogEntry> SysMonitor::readLogFile(const std::string& filePath, size_t maxLines) const {
#if defined(_WIN32)
    return read_log_file_win(filePath, maxLines);
#elif defined(__linux__)
    return read_log_file_linux(filePath, maxLines);
#elif defined(__APPLE__)
    return read_log_file_mac(filePath, maxLines);
#else
    throw std::runtime_error("Unsupported platform for Log file reading.");
#endif
}
