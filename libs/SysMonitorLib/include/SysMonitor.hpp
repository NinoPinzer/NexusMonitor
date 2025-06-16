#pragma once

#include "DataTypes.hpp"

class SysMonitor {
public:
    SysMonitor();
    ~SysMonitor();

    CpuStats getCpuStats() const;
    MemoryStats getMemoryStats() const;
    DiskIoStats getDiskIoStats(const std::string& diskName) const;
    std::vector<NetworkIoStats> getNetworkIoStats() const;
    std::vector<ProcessInfo> getProcessList() const;
    ServiceStatus getServiceStatus(const std::string& serviceName) const;
    std::vector<LogEntry> readLogFile(const std::string& filePath, size_t maxLines) const;

    // TODO : implement
    // double getSystemUptime() const;
    //std::vector<DeviceInfo> getDeviceInfo() const;
};