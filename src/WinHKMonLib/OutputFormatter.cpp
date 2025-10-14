#include "WinHKMonLib/OutputFormatter.h"
#include <sstream>
#include <iomanip>
#include <ctime>

namespace WinHKMon {

namespace {

// Escape string for JSON
std::string escapeJson(const std::string& str) {
    std::string escaped;
    escaped.reserve(str.size());
    
    for (char c : str) {
        switch (c) {
            case '"':  escaped += "\\\""; break;
            case '\\': escaped += "\\\\"; break;
            case '\n': escaped += "\\n"; break;
            case '\r': escaped += "\\r"; break;
            case '\t': escaped += "\\t"; break;
            case '\b': escaped += "\\b"; break;
            case '\f': escaped += "\\f"; break;
            default:   escaped += c; break;
        }
    }
    
    return escaped;
}

// Escape string for CSV (RFC 4180)
std::string escapeCsv(const std::string& str) {
    bool needsQuoting = (str.find(',') != std::string::npos ||
                         str.find('"') != std::string::npos ||
                         str.find('\n') != std::string::npos);
    
    if (!needsQuoting) {
        return str;
    }
    
    std::string escaped = "\"";
    for (char c : str) {
        if (c == '"') {
            escaped += "\"\"";  // Double quotes
        } else {
            escaped += c;
        }
    }
    escaped += "\"";
    
    return escaped;
}

// Format bytes as human-readable size
std::string formatBytes(uint64_t bytes) {
    const uint64_t KB = 1024;
    const uint64_t MB = KB * 1024;
    const uint64_t GB = MB * 1024;
    
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1);
    
    if (bytes >= GB) {
        oss << (static_cast<double>(bytes) / GB) << " GB";
    } else if (bytes >= MB) {
        oss << (static_cast<double>(bytes) / MB) << " MB";
    } else if (bytes >= KB) {
        oss << (static_cast<double>(bytes) / KB) << " KB";
    } else {
        oss << bytes << " B";
    }
    
    return oss.str();
}

// Format frequency in GHz
std::string formatFrequency(uint64_t mhz) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1);
    oss << (static_cast<double>(mhz) / 1000.0) << " GHz";
    return oss.str();
}

// Format bytes per second as transfer rate
std::string formatBytesPerSec(uint64_t bytesPerSec) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1);
    
    if (bytesPerSec >= 1000000000) {
        oss << (static_cast<double>(bytesPerSec) / 1000000000.0) << " GB/s";
    } else if (bytesPerSec >= 1000000) {
        oss << (static_cast<double>(bytesPerSec) / 1000000.0) << " MB/s";
    } else if (bytesPerSec >= 1000) {
        oss << (static_cast<double>(bytesPerSec) / 1000.0) << " KB/s";
    } else {
        oss << bytesPerSec << " B/s";
    }
    
    return oss.str();
}

// Format bits per second as network speed
std::string formatBitsPerSec(uint64_t bitsPerSec) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1);
    
    if (bitsPerSec >= 1000000000) {
        oss << (static_cast<double>(bitsPerSec) / 1000000000.0) << " Gbps";
    } else if (bitsPerSec >= 1000000) {
        oss << (static_cast<double>(bitsPerSec) / 1000000.0) << " Mbps";
    } else if (bitsPerSec >= 1000) {
        oss << (static_cast<double>(bitsPerSec) / 1000.0) << " Kbps";
    } else {
        oss << bitsPerSec << " bps";
    }
    
    return oss.str();
}

// Get current timestamp as ISO 8601 string
std::string getTimestampString() {
    auto now = std::time(nullptr);
    std::tm tm{};
    
#ifdef _WIN32
    // Use secure version on Windows
    gmtime_s(&tm, &now);
#else
    // Use standard version on other platforms
    tm = *std::gmtime(&now);
#endif
    
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
    return oss.str();
}

}  // anonymous namespace

std::string formatText(const SystemMetrics& metrics, bool singleLine, const CliOptions& options) {
    std::ostringstream output;
    output << std::fixed << std::setprecision(1);
    
    const char* separator = singleLine ? "  " : "\n";
    // Use ASCII characters for Windows console compatibility (UTF-8 arrows don't display correctly)
    const char* arrowUp = "<";      // < = Read (for disk) / In (for network)
    const char* arrowDown = ">";    // > = Write (for disk) / Out (for network)
    
    // CPU
    if (metrics.cpu) {
        if (singleLine) {
            output << "CPU:" << metrics.cpu->totalUsagePercent << "%@"
                   << formatFrequency(metrics.cpu->averageFrequencyMhz);
        } else {
            output << "CPU:  " << metrics.cpu->totalUsagePercent << "%  "
                   << formatFrequency(metrics.cpu->averageFrequencyMhz);
        }
        output << separator;
    }
    
    // Memory
    if (metrics.memory) {
        uint64_t availableMB = metrics.memory->availablePhysicalBytes / (1024 * 1024);
        if (singleLine) {
            output << "RAM:" << availableMB << "M";
        } else {
            output << "RAM:  " << availableMB << " MB available ("
                   << metrics.memory->usagePercent << "% used)";
        }
        output << separator;
    }
    
    // Disk Space (DISK metric)
    if (metrics.disks && options.showDiskSpace) {
        for (const auto& disk : *metrics.disks) {
            if (singleLine) {
                output << "DISK:" << disk.deviceName << ":"
                       << formatBytes(disk.usedBytes) << "/"
                       << formatBytes(disk.totalSizeBytes);
            } else {
                double usedPercent = (disk.totalSizeBytes > 0) 
                    ? (static_cast<double>(disk.usedBytes) / disk.totalSizeBytes * 100.0) 
                    : 0.0;
                output << "DISK: " << disk.deviceName << " "
                       << formatBytes(disk.usedBytes) << " / "
                       << formatBytes(disk.totalSizeBytes) << " "
                       << "(" << std::fixed << std::setprecision(1) << usedPercent << "% used, "
                       << formatBytes(disk.freeBytes) << " free)";
            }
            output << separator;
        }
    }
    
    // Disk I/O (IO metric)
    if (metrics.disks && options.showDiskIO) {
        for (const auto& disk : *metrics.disks) {
            if (singleLine) {
                output << "IO:" << disk.deviceName << ":"
                       << formatBytesPerSec(disk.bytesReadPerSec) << arrowUp
                       << formatBytesPerSec(disk.bytesWrittenPerSec) << arrowDown;
            } else {
                output << "IO:   " << disk.deviceName << " "
                       << arrowUp << " " << formatBytesPerSec(disk.bytesReadPerSec) << "  "
                       << arrowDown << " " << formatBytesPerSec(disk.bytesWrittenPerSec)
                       << "  (" << disk.percentBusy << "% busy)";
            }
            output << separator;
        }
    }
    
    // Network
    if (metrics.network) {
        for (const auto& iface : *metrics.network) {
            if (singleLine) {
                output << "NET:" << iface.name << ":"
                       << formatBitsPerSec(iface.inBytesPerSec * 8) << arrowUp
                       << formatBitsPerSec(iface.outBytesPerSec * 8) << arrowDown;
            } else {
                output << "NET:  " << iface.name << " "
                       << arrowUp << " " << formatBitsPerSec(iface.inBytesPerSec * 8) << "  "
                       << arrowDown << " " << formatBitsPerSec(iface.outBytesPerSec * 8);
                if (iface.linkSpeedBitsPerSec > 0) {
                    output << "  (" << formatBitsPerSec(iface.linkSpeedBitsPerSec) << " link)";
                }
            }
            output << separator;
        }
    }
    
    // Temperature
    if (metrics.temperature) {
        if (singleLine) {
            output << "TEMP:" << metrics.temperature->maxCpuTempCelsius << "°C";
        } else {
            output << "TEMP: CPU " << metrics.temperature->maxCpuTempCelsius << "°C";
            if (metrics.temperature->avgCpuTempCelsius) {
                output << "  (avg: " << *metrics.temperature->avgCpuTempCelsius << "°C)";
            }
        }
        output << separator;
    }
    
    std::string result = output.str();
    
    // If no metrics were output, provide minimal feedback
    if (result.empty()) {
        return singleLine ? "(no metrics)" : "(no metrics)\n";
    }
    
    // Remove trailing separator for single-line mode
    if (singleLine && !result.empty() && result.back() == ' ') {
        result.pop_back();
        result.pop_back();  // Remove both spaces
    }
    
    return result;
}

std::string formatJson(const SystemMetrics& metrics, const CliOptions& options) {
    // Note: options parameter is for API consistency; JSON always includes all available fields
    (void)options;
    
    std::ostringstream json;
    json << std::fixed << std::setprecision(1);
    
    json << "{\n";
    json << "  \"schemaVersion\": \"1.0\",\n";
    json << "  \"timestamp\": \"" << getTimestampString() << "\"";
    
    // CPU
    if (metrics.cpu) {
        json << ",\n  \"cpu\": {\n";
        json << "    \"totalUsagePercent\": " << metrics.cpu->totalUsagePercent << ",\n";
        json << "    \"averageFrequencyMhz\": " << metrics.cpu->averageFrequencyMhz;
        
        if (!metrics.cpu->cores.empty()) {
            json << ",\n    \"cores\": [\n";
            for (size_t i = 0; i < metrics.cpu->cores.size(); i++) {
                const auto& core = metrics.cpu->cores[i];
                json << "      {\"id\": " << core.coreId
                     << ", \"usagePercent\": " << core.usagePercent
                     << ", \"frequencyMhz\": " << core.frequencyMhz << "}";
                if (i < metrics.cpu->cores.size() - 1) {
                    json << ",";
                }
                json << "\n";
            }
            json << "    ]";
        }
        
        json << "\n  }";
    }
    
    // Memory
    if (metrics.memory) {
        json << ",\n  \"memory\": {\n";
        json << "    \"totalMB\": " << (metrics.memory->totalPhysicalBytes / (1024 * 1024)) << ",\n";
        json << "    \"availableMB\": " << (metrics.memory->availablePhysicalBytes / (1024 * 1024)) << ",\n";
        json << "    \"usedMB\": " << (metrics.memory->usedPhysicalBytes / (1024 * 1024)) << ",\n";
        json << "    \"usagePercent\": " << metrics.memory->usagePercent << ",\n";
        json << "    \"pageFile\": {\n";
        json << "      \"totalMB\": " << (metrics.memory->totalPageFileBytes / (1024 * 1024)) << ",\n";
        json << "      \"usedMB\": " << (metrics.memory->usedPageFileBytes / (1024 * 1024)) << ",\n";
        json << "      \"usagePercent\": " << metrics.memory->pageFilePercent << "\n";
        json << "    }\n";
        json << "  }";
    }
    
    // Disks (includes both space and I/O data)
    if (metrics.disks && !metrics.disks->empty()) {
        json << ",\n  \"disks\": [\n";
        for (size_t i = 0; i < metrics.disks->size(); i++) {
            const auto& disk = (*metrics.disks)[i];
            json << "    {\n";
            json << "      \"deviceName\": \"" << escapeJson(disk.deviceName) << "\",\n";
            // Space information (DISK metric)
            json << "      \"totalSizeBytes\": " << disk.totalSizeBytes << ",\n";
            json << "      \"usedBytes\": " << disk.usedBytes << ",\n";
            json << "      \"freeBytes\": " << disk.freeBytes << ",\n";
            // I/O information (IO metric)
            json << "      \"bytesReadPerSec\": " << disk.bytesReadPerSec << ",\n";
            json << "      \"bytesWrittenPerSec\": " << disk.bytesWrittenPerSec << ",\n";
            json << "      \"percentBusy\": " << disk.percentBusy << "\n";
            json << "    }";
            if (i < metrics.disks->size() - 1) {
                json << ",";
            }
            json << "\n";
        }
        json << "  ]";
    }
    
    // Network
    if (metrics.network && !metrics.network->empty()) {
        json << ",\n  \"network\": [\n";
        for (size_t i = 0; i < metrics.network->size(); i++) {
            const auto& iface = (*metrics.network)[i];
            json << "    {\n";
            json << "      \"name\": \"" << escapeJson(iface.name) << "\",\n";
            json << "      \"description\": \"" << escapeJson(iface.description) << "\",\n";
            json << "      \"isConnected\": " << (iface.isConnected ? "true" : "false") << ",\n";
            json << "      \"linkSpeedBitsPerSec\": " << iface.linkSpeedBitsPerSec << ",\n";
            json << "      \"inBytesPerSec\": " << iface.inBytesPerSec << ",\n";
            json << "      \"outBytesPerSec\": " << iface.outBytesPerSec << "\n";
            json << "    }";
            if (i < metrics.network->size() - 1) {
                json << ",";
            }
            json << "\n";
        }
        json << "  ]";
    }
    
    // Temperature
    if (metrics.temperature) {
        json << ",\n  \"temperature\": {\n";
        json << "    \"maxCpuTempCelsius\": " << metrics.temperature->maxCpuTempCelsius;
        
        if (metrics.temperature->avgCpuTempCelsius) {
            json << ",\n    \"avgCpuTempCelsius\": " << *metrics.temperature->avgCpuTempCelsius;
        }
        
        json << "\n  }";
    }
    
    json << "\n}";
    
    return json.str();
}

std::string formatCsv(const SystemMetrics& metrics, bool includeHeader, const CliOptions& options) {
    std::ostringstream csv;
    
    if (includeHeader) {
        csv << "timestamp,cpu_percent,cpu_mhz,ram_available_mb,ram_percent";
        
        if (metrics.disks && options.showDiskSpace) {
            csv << ",disk_name,disk_used_gb,disk_total_gb,disk_free_gb,disk_used_percent";
        }
        
        if (metrics.disks && options.showDiskIO) {
            csv << ",disk_name,disk_read_mbps,disk_write_mbps,disk_busy_percent";
        }
        
        if (metrics.network) {
            csv << ",net_interface,net_recv_mbps,net_sent_mbps";
        }
        
        if (metrics.temperature) {
            csv << ",temp_celsius";
        }
        
        csv << "\n";
    }
    
    // Data row
    csv << getTimestampString() << ",";
    
    // CPU
    if (metrics.cpu) {
        csv << metrics.cpu->totalUsagePercent << ","
            << metrics.cpu->averageFrequencyMhz << ",";
    } else {
        csv << ",,";
    }
    
    // Memory
    if (metrics.memory) {
        csv << (metrics.memory->availablePhysicalBytes / (1024 * 1024)) << ","
            << metrics.memory->usagePercent;
    } else {
        csv << ",";
    }
    
    // Disk Space (first disk only for CSV simplicity)
    if (metrics.disks && !metrics.disks->empty() && options.showDiskSpace) {
        const auto& disk = (*metrics.disks)[0];
        double usedGB = disk.usedBytes / (1024.0 * 1024.0 * 1024.0);
        double totalGB = disk.totalSizeBytes / (1024.0 * 1024.0 * 1024.0);
        double freeGB = disk.freeBytes / (1024.0 * 1024.0 * 1024.0);
        double usedPercent = (disk.totalSizeBytes > 0) 
            ? (static_cast<double>(disk.usedBytes) / disk.totalSizeBytes * 100.0) 
            : 0.0;
        csv << "," << escapeCsv(disk.deviceName)
            << "," << std::fixed << std::setprecision(2) << usedGB
            << "," << totalGB
            << "," << freeGB
            << "," << std::setprecision(1) << usedPercent;
    }
    
    // Disk I/O (first disk only for CSV simplicity)
    if (metrics.disks && !metrics.disks->empty() && options.showDiskIO) {
        const auto& disk = (*metrics.disks)[0];
        csv << "," << escapeCsv(disk.deviceName)
            << "," << std::fixed << std::setprecision(2) 
            << (disk.bytesReadPerSec / (1024.0 * 1024.0))
            << "," << (disk.bytesWrittenPerSec / (1024.0 * 1024.0))
            << "," << std::setprecision(1) << disk.percentBusy;
    }
    
    // Network (first interface only for CSV simplicity)
    if (metrics.network && !metrics.network->empty()) {
        const auto& iface = (*metrics.network)[0];
        csv << "," << escapeCsv(iface.name)
            << "," << (iface.inBytesPerSec * 8.0 / 1000000.0)  // Convert to Mbps
            << "," << (iface.outBytesPerSec * 8.0 / 1000000.0);
    }
    
    // Temperature
    if (metrics.temperature) {
        csv << "," << metrics.temperature->maxCpuTempCelsius;
    }
    
    csv << "\n";
    
    return csv.str();
}

}  // namespace WinHKMon

