#pragma once

#include <string>
#include <vector>
#include <map>
#include <chrono> // Für Zeitstempel

// Basis-Strukturen für Metriken
struct CpuStats {
    double usagePercent;
    double idlePercent;
    double userPercent;
    double systemPercent;
    // Weitere CPU-spezifische Daten (z.B. per Core, Frequenz)
    std::map<std::string, double> coreUsages;
    long long totalCycles; // Für Delta-Berechnungen
    long long idleCycles;
    long long userCycles;
    long long systemCycles;
    long long interruptCycles;
    std::chrono::system_clock::time_point timestamp;
};

struct MemoryStats {
    long long totalBytes;
    long long usedBytes;
    long long freeBytes;
    double usagePercent;
    long long swapTotalBytes;
    long long swapUsedBytes;
    long long swapFreeBytes;
    std::chrono::system_clock::time_point timestamp;
};

struct DiskIoStats {
    std::string diskName;
    long long bytesRead;
    long long bytesWritten;
    long long readsCompleted;
    long long writesCompleted;
    std::chrono::system_clock::time_point timestamp;
};

struct NetworkIoStats {
    std::string interfaceName;
    long long bytesSent;
    long long bytesReceived;
    long long packetsSent;
    long long packetsReceived;
    std::chrono::system_clock::time_point timestamp;
};

enum ProcessStatus {
    Running,
    Sleeping,
    Stopped,
    Zombie,
    ProcUnknown
};

struct ProcessInfo {
    int pid;
    std::string name;
    std::string user;
    double cpuPercent;
    long long memoryBytes;
    ProcessStatus status;
    std::string commandLine;
    std::chrono::system_clock::time_point timestamp;
};

enum ServiceStateType {
    ServiceRunning,
    ServiceStopped,
    ServicePaused,
    ServiceStartPending,
    ServiceStopPending,
    ServiceContinuePending,
    ServicePausePending,
    ServiceUnknown
};

struct ServiceStatus {
    std::string name;
    std::string displayName;
    ServiceStateType state;
    std::chrono::system_clock::time_point timestamp;
};

struct LogEntry {
    std::chrono::system_clock::time_point timestamp;
    std::string level;
    std::string message;
    std::string sourceFile;
};