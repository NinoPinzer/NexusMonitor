#pragma once

#include "DataTypes.hpp" // Für die Metrik-Strukturen
#include <string>
#include <vector>

#if defined(__linux__)
CpuStats get_cpu_stats_linux();
MemoryStats get_memory_stats_linux();
DiskIoStats get_disk_io_stats_linux(const std::string& diskName);
std::vector<NetworkIoStats> get_network_io_stats_linux();
std::vector<ProcessInfo> get_process_list_linux();
ServiceStatus get_service_status_linux(const std::string& serviceName);
std::vector<LogEntry> read_log_file_linux(const std::string& filePath, size_t maxLines);
#endif

#if defined(_WIN32)
CpuStats get_cpu_stats_win();
MemoryStats get_memory_stats_win();
DiskIoStats get_disk_io_stats_win(const std::string& diskName);
std::vector<NetworkIoStats> get_network_io_stats_win();
std::vector<ProcessInfo> get_process_list_win();
ServiceStatus get_service_status_win(const std::string& serviceName);
std::vector<LogEntry> read_log_file_win(const std::string& filePath, size_t maxLines);
#endif

#if defined(__APPLE__)
CpuStats get_cpu_stats_mac();
MemoryStats get_memory_stats_mac();
// ... und so weiter für macOS
#endif