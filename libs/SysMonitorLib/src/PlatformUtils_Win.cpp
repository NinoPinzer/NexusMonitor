#define _WIN32_WINNT 0x0600
#define _WIN32_IE 0x0700

#include "PlatformUtils.hpp"
#include <windows.h>
#include <Pdh.h>
#include <Psapi.h>
#include <Iphlpapi.h>
#include <winreg.h>
#include <Shlwapi.h>
#include <winsvc.h>

#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <numeric>
#include <fstream>
#include <sstream>

#pragma comment(lib, "pdh.lib")
#pragma comment(lib, "psapi.lib")
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "advapi32.lib")

#if defined(_WIN32)

// Globale Variablen für CPU-Messung (für Delta-Berechnung)
static ULARGE_INTEGER lastIdleTime = {0};
static ULARGE_INTEGER lastKernelTime = {0};
static ULARGE_INTEGER lastUserTime = {0};
static bool firstCpuCall = true;

// Hilfsfunktion zur Umwandlung von FILETIME in ULARGE_INTEGER
ULARGE_INTEGER to_ularge(const FILETIME& ft) {
    ULARGE_INTEGER u;
    u.LowPart = ft.dwLowDateTime;
    u.HighPart = ft.dwHighDateTime;
    return u;
}

CpuStats get_cpu_stats_win() {
    CpuStats stats = {}; // Wichtig: Alle Mitglieder auf 0/Standardwerte initialisieren
    stats.timestamp = std::chrono::system_clock::now();

    FILETIME idleTime, kernelTime, userTime;
    if (!GetSystemTimes(&idleTime, &kernelTime, &userTime)) {
        std::cerr << "Error getting system times: " << GetLastError() << std::endl;
        return stats; // Rückgabe von Standard-initialisierten Stats
    }

    ULARGE_INTEGER currentIdleTime = to_ularge(idleTime);
    ULARGE_INTEGER currentKernelTime = to_ularge(kernelTime);
    ULARGE_INTEGER currentUserTime = to_ularge(userTime);

    if (firstCpuCall) {
        lastIdleTime = currentIdleTime;
        lastKernelTime = currentKernelTime;
        lastUserTime = currentUserTime;
        firstCpuCall = false;
        // Beim ersten Aufruf können wir keine sinnvolle Nutzung berechnen.
        // Setze sie auf 0 und der Agent muss dies berücksichtigen oder den nächsten Wert abwarten.
        stats.usagePercent = 0.0;
        stats.idlePercent = 100.0;
        return stats;
    }

    ULONGLONG idle = currentIdleTime.QuadPart - lastIdleTime.QuadPart;
    ULONGLONG kernel = currentKernelTime.QuadPart - lastKernelTime.QuadPart;
    ULONGLONG user = currentUserTime.QuadPart - lastUserTime.QuadPart;

    // Gesamtzeit (Kernel + User) abzüglich Leerlaufzeit
    ULONGLONG total = kernel + user;

    if (total > 0) {
        stats.idlePercent = (static_cast<double>(idle) / total) * 100.0;
        stats.usagePercent = 100.0 - stats.idlePercent;
        stats.userPercent = (static_cast<double>(user) / total) * 100.0;
        // Kernel Time enthält auch Idle Time, daher müssen wir idle abziehen für Systemzeit
        stats.systemPercent = (static_cast<double>(kernel - idle) / total) * 100.0;
    } else {
        // Bei sehr kurzen Intervallen oder Fehler
        stats.usagePercent = 0.0;
        stats.idlePercent = 100.0;
        stats.userPercent = 0.0;
        stats.systemPercent = 0.0;
    }

    // Speichern für den nächsten Aufruf
    lastIdleTime = currentIdleTime;
    lastKernelTime = currentKernelTime;
    lastUserTime = currentUserTime;

    return stats;
}


MemoryStats get_memory_stats_win() {
    MemoryStats stats = {};
    stats.timestamp = std::chrono::system_clock::now();

    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);

    if (GlobalMemoryStatusEx(&memInfo)) {
        stats.totalBytes = memInfo.ullTotalPhys;
        stats.usedBytes = memInfo.ullTotalPhys - memInfo.ullAvailPhys;
        stats.freeBytes = memInfo.ullAvailPhys;
        stats.usagePercent = static_cast<double>(memInfo.dwMemoryLoad); // dwMemoryLoad ist bereits in Prozent

        stats.swapTotalBytes = memInfo.ullTotalPageFile;
        stats.swapUsedBytes = memInfo.ullTotalPageFile - memInfo.ullAvailPageFile;
        stats.swapFreeBytes = memInfo.ullAvailPageFile;
    } else {
        std::cerr << "Error getting memory status: " << GetLastError() << std::endl;
    }
    return stats;
}

DiskIoStats get_disk_io_stats_win(const std::string& driveLetter) {
    DiskIoStats stats = {};
    stats.diskName = driveLetter;
    stats.timestamp = std::chrono::system_clock::now();

    // Für detaillierte Disk-I/O-Statistiken sind Performance Counters (Pdh) oder WMI am besten.
    // Ein einfacher Ansatz ist hier nicht wirklich aussagekräftig für "Reads/Writes completed" oder "bytes".
    // Hier ist ein Platzhalter, der mit Pdh gefüllt werden müsste.

    PDH_HQUERY hQuery = NULL;
    PDH_HCOUNTER hCounterRead = NULL;
    PDH_HCOUNTER hCounterWrite = NULL;

    if (PdhOpenQuery(NULL, 0, &hQuery) == ERROR_SUCCESS) {
        std::string readPath = "\\PhysicalDisk(_Total)\\% Disk Read Bytes/sec";
        std::string writePath = "\\PhysicalDisk(_Total)\\% Disk Write Bytes/sec";

        // Um einen spezifischen Laufwerksbuchstaben zu überwachen, müsste der Counter-Pfad angepasst werden
        // z.B. "\\PhysicalDisk(0 C:)\\% Disk Read Bytes/sec" -> dies erfordert Enum-Funktionen
        // Für Einfachheit nehmen wir hier _Total.
        if (PdhAddCounterA(hQuery, readPath.c_str(), 0, &hCounterRead) == ERROR_SUCCESS &&
            PdhAddCounterA(hQuery, writePath.c_str(), 0, &hCounterWrite) == ERROR_SUCCESS) {

            PdhCollectQueryData(hQuery); // Erster Aufruf zum Initialisieren

            stats.bytesRead = 0;
            stats.bytesWritten = 0;
            stats.readsCompleted = 0;
            stats.writesCompleted = 0;
        }
        PdhCloseQuery(hQuery);
    }
    return stats;
}

std::vector<NetworkIoStats> get_network_io_stats_win() {
    std::vector<NetworkIoStats> stats_list;

    PIP_ADAPTER_INFO pAdapterInfo;
    PIP_ADAPTER_INFO pAdapter = NULL;
    DWORD dwRetVal = 0;
    ULONG ulOutBufLen = sizeof(IP_ADAPTER_INFO);

    pAdapterInfo = (IP_ADAPTER_INFO*)malloc(sizeof(IP_ADAPTER_INFO));
    if (pAdapterInfo == NULL) {
        std::cerr << "Error allocating memory for adapter info" << std::endl;
        return stats_list;
    }

    // Stelle sicher, dass der Puffer groß genug ist
    if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW) {
        free(pAdapterInfo);
        pAdapterInfo = (IP_ADAPTER_INFO*)malloc(ulOutBufLen);
        if (pAdapterInfo == NULL) {
            std::cerr << "Error re-allocating memory for adapter info" << std::endl;
            return stats_list;
        }
    }

    if ((dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen)) == NO_ERROR) {
        pAdapter = pAdapterInfo;
        while (pAdapter) {
            NetworkIoStats stats = {};
            stats.interfaceName = pAdapter->Description;
            stats.timestamp = std::chrono::system_clock::now(); // <-- Timestamp hier setzen!

            // --- HIER IST DIE KORREKTUR ---
            // Entferne alle Zeilen, die MIB_IF_ROW2 und GetIfEntry2 betreffen.
            // Wir verwenden ausschließlich MIB_IFROW und GetIfEntry.

            MIB_IFROW legacyIfRow; // Behalte diese Deklaration
            legacyIfRow.dwIndex = pAdapter->Index; // Setze den Index

            // Überprüfe den Rückgabewert von GetIfEntry
            if (GetIfEntry(&legacyIfRow) == NO_ERROR) {
                stats.bytesReceived = legacyIfRow.dwInOctets;
                stats.bytesSent = legacyIfRow.dwOutOctets;
                // Achte auf die korrekte Schreibweise: dwInNUcastPkts und dwOutNUcastPkts
                stats.packetsReceived = legacyIfRow.dwInUcastPkts + legacyIfRow.dwInNUcastPkts;
                stats.packetsSent = legacyIfRow.dwOutUcastPkts + legacyIfRow.dwOutNUcastPkts;
            } else {
                std::cerr << "Error getting network interface stats for " << pAdapter->Description << ": " << GetLastError() << std::endl;
            }

            stats_list.push_back(stats);
            pAdapter = pAdapter->Next;
        }
    } else if (dwRetVal == ERROR_NO_DATA) {
        // Keine Adapter gefunden
    } else {
        std::cerr << "Error getting adapters info: " << dwRetVal << std::endl;
    }

    if (pAdapterInfo) {
        free(pAdapterInfo);
    }
    return stats_list;
}

std::vector<ProcessInfo> get_process_list_win() {
    std::vector<ProcessInfo> processes;
    DWORD aProcesses[1024], cbNeeded, cProcesses;

    if (!EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded)) {
        std::cerr << "Error enumerating processes: " << GetLastError() << std::endl;
        return processes;
    }

    cProcesses = cbNeeded / sizeof(DWORD);

    for (unsigned int i = 0; i < cProcesses; i++) {
        if (aProcesses[i] == 0) continue;

        ProcessInfo pInfo = {};
        pInfo.pid = aProcesses[i];
        pInfo.timestamp = std::chrono::system_clock::now();

        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, aProcesses[i]);

        if (hProcess == NULL) {
            // std::cerr << "Warning: Could not open process " << aProcesses[i] << ": " << GetLastError() << std::endl;
            continue;
        }

        HMODULE hMod;
        DWORD cbModNeeded;
        if (EnumProcessModules(hProcess, &hMod, sizeof(hMod), &cbModNeeded)) {
            char szProcessName[MAX_PATH];
            if (GetModuleBaseNameA(hProcess, hMod, szProcessName, sizeof(szProcessName) / sizeof(char))) {
                pInfo.name = szProcessName;
            }
        }

        // Get memory usage
        PROCESS_MEMORY_COUNTERS pmc;
        if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc))) {
            pInfo.memoryBytes = pmc.WorkingSetSize;
        }

        pInfo.status = ProcessStatus::Running;

        processes.push_back(pInfo);
        CloseHandle(hProcess);
    }
    return processes;
}

ServiceStatus get_service_status_win(const std::string& serviceName) {
    ServiceStatus status = {};
    status.name = serviceName;
    status.displayName = serviceName;
    status.timestamp = std::chrono::system_clock::now();
    status.state = ServiceStateType::ServiceUnknown;

    SC_HANDLE scm = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (scm == NULL) {
        std::cerr << "Error opening Service Control Manager: " << GetLastError() << std::endl;
        return status;
    }

    SC_HANDLE svc = OpenServiceA(scm, serviceName.c_str(), SERVICE_QUERY_STATUS);
    if (svc == NULL) {
        DWORD error = GetLastError();
        if (error == ERROR_SERVICE_DOES_NOT_EXIST) {

        } else {
            std::cerr << "Error opening service " << serviceName << ": " << error << std::endl;
        }
        CloseServiceHandle(scm);
        return status;
    }

    SERVICE_STATUS_PROCESS ssp;
    DWORD bytesNeeded;
if (QueryServiceStatusEx(svc, SC_STATUS_PROCESS_INFO, (LPBYTE)&ssp, sizeof(SERVICE_STATUS_PROCESS), &bytesNeeded)) {
        switch (ssp.dwCurrentState) {
            case SERVICE_RUNNING:
                status.state = ServiceStateType::ServiceRunning;
                break;
            case SERVICE_STOPPED:
                status.state = ServiceStateType::ServiceStopped;
                break;
            case SERVICE_PAUSED:
                status.state = ServiceStateType::ServicePaused;
                break;
            case SERVICE_START_PENDING:
                status.state = ServiceStateType::ServiceStartPending;
                break;
            case SERVICE_STOP_PENDING:
                status.state = ServiceStateType::ServiceStopPending;
                break;
            case SERVICE_CONTINUE_PENDING:
                status.state = ServiceStateType::ServiceContinuePending;
                break;
            case SERVICE_PAUSE_PENDING:
                status.state = ServiceStateType::ServicePausePending;
                break;
            default:
                status.state = ServiceStateType::ServiceUnknown;
                break;
        }
    } else {
        std::cerr << "Error querying service status for " << serviceName << ": " << GetLastError() << std::endl;
    }

    CloseServiceHandle(svc);
    CloseServiceHandle(scm);
    return status;
}

std::vector<LogEntry> read_log_file_win(const std::string& filePath, size_t maxLines) {
    std::vector<LogEntry> entries;
    if (!PathFileExistsA(filePath.c_str())) {
        std::cerr << "Log file does not exist: " << filePath << std::endl;
        return entries;
    }

    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Error opening log file: " << filePath << std::endl;
        return entries;
    }

    std::string line;
    std::vector<std::string> lines;
    while (std::getline(file, line)) {
        lines.push_back(line);
    }

    // Lese die letzten maxLines
    size_t startLine = 0;
    if (lines.size() > maxLines) {
        startLine = lines.size() - maxLines;
    }

    for (size_t i = startLine; i < lines.size(); ++i) {
        LogEntry entry;
        entry.timestamp = std::chrono::system_clock::now(); // Platzhalter, da Zeitstempel geparst werden müsste
        entry.message = lines[i];
        entry.sourceFile = filePath;
        entry.level = "INFO"; // Platzhalter, da Level geparst werden müsste
        entries.push_back(entry);
    }

    return entries;
}

#endif // _WIN32