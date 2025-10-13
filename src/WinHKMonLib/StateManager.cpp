#include "WinHKMonLib/StateManager.h"
#include <fstream>
#include <sstream>
#include <algorithm>

namespace WinHKMon {

StateManager::StateManager(const std::string& appName) : appName_(appName) {
}

std::filesystem::path StateManager::getStatePath() const {
    auto tempPath = std::filesystem::temp_directory_path();
    return tempPath / (appName_ + ".dat");
}

bool StateManager::validateVersion(const std::string& version) const {
    // Accept version 1.x
    return version.substr(0, 2) == "1.";
}

std::string StateManager::sanitizeKey(const std::string& key) const {
    // Replace only characters that would break the file format
    // Spaces are OK in key names since we use the last underscore as delimiter
    std::string sanitized = key;
    std::replace_if(sanitized.begin(), sanitized.end(),
                    [](char c) { return c == '\t' || c == '\n' || c == '\r'; },
                    '_');
    return sanitized;
}

bool StateManager::load(SystemMetrics& metrics, uint64_t& timestamp) {
    auto statePath = getStatePath();
    
    if (!std::filesystem::exists(statePath)) {
        return false;  // First run, no state file
    }
    
    std::ifstream file(statePath);
    if (!file.is_open()) {
        return false;
    }
    
    std::string line;
    std::string version;
    
    // Read version
    if (!std::getline(file, line)) {
        return false;  // Empty file
    }
    
    if (line.substr(0, 8) != "VERSION ") {
        return false;  // Invalid format
    }
    
    version = line.substr(8);
    if (!validateVersion(version)) {
        return false;  // Incompatible version
    }
    
    // Read timestamp
    if (!std::getline(file, line)) {
        return false;
    }
    
    if (line.substr(0, 10) != "TIMESTAMP ") {
        return false;
    }
    
    try {
        timestamp = std::stoull(line.substr(10));
    } catch (...) {
        return false;
    }
    
    // Read metrics
    std::vector<InterfaceStats> networkInterfaces;
    std::vector<DiskStats> disks;
    
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        
        std::istringstream iss(line);
        std::string key;
        uint64_t value;
        
        if (!(iss >> key >> value)) {
            continue;  // Skip malformed lines
        }
        
        // Parse key to extract type, device, and field
        if (key.substr(0, 8) == "NETWORK_") {
            size_t lastUnderscore = key.rfind('_');
            if (lastUnderscore == std::string::npos || lastUnderscore <= 8) continue;
            
            std::string interfaceName = key.substr(8, lastUnderscore - 8);
            std::string field = key.substr(lastUnderscore + 1);
            
            // Find or create interface
            auto it = std::find_if(networkInterfaces.begin(), networkInterfaces.end(),
                                   [&interfaceName](const InterfaceStats& iface) {
                                       return iface.name == interfaceName;
                                   });
            
            if (it == networkInterfaces.end()) {
                InterfaceStats iface;
                iface.name = interfaceName;
                iface.isConnected = false;
                iface.linkSpeedBitsPerSec = 0;
                iface.inBytesPerSec = 0;
                iface.outBytesPerSec = 0;
                iface.totalInOctets = 0;
                iface.totalOutOctets = 0;
                networkInterfaces.push_back(iface);
                it = networkInterfaces.end() - 1;
            }
            
            if (field == "IN") {
                it->totalInOctets = value;
            } else if (field == "OUT") {
                it->totalOutOctets = value;
            }
        }
        else if (key.substr(0, 5) == "DISK_") {
            size_t lastUnderscore = key.rfind('_');
            if (lastUnderscore == std::string::npos || lastUnderscore <= 5) continue;
            
            std::string deviceName = key.substr(5, lastUnderscore - 5);
            std::string field = key.substr(lastUnderscore + 1);
            
            // Find or create disk
            auto it = std::find_if(disks.begin(), disks.end(),
                                   [&deviceName](const DiskStats& disk) {
                                       return disk.deviceName == deviceName;
                                   });
            
            if (it == disks.end()) {
                DiskStats disk;
                disk.deviceName = deviceName;
                disk.totalSizeBytes = 0;
                disk.bytesReadPerSec = 0;
                disk.bytesWrittenPerSec = 0;
                disk.percentBusy = 0.0;
                disk.totalBytesRead = 0;
                disk.totalBytesWritten = 0;
                disks.push_back(disk);
                it = disks.end() - 1;
            }
            
            if (field == "READ") {
                it->totalBytesRead = value;
            } else if (field == "WRITE") {
                it->totalBytesWritten = value;
            }
        }
    }
    
    // Populate metrics
    if (!networkInterfaces.empty()) {
        metrics.network = networkInterfaces;
    }
    
    if (!disks.empty()) {
        metrics.disks = disks;
    }
    
    return true;
}

bool StateManager::save(const SystemMetrics& metrics) {
    auto statePath = getStatePath();
    
    std::ofstream file(statePath, std::ios::trunc);
    if (!file.is_open()) {
        return false;
    }
    
    // Write version
    file << "VERSION " << VERSION << "\n";
    
    // Write timestamp
    file << "TIMESTAMP " << metrics.timestamp << "\n";
    
    // Write network interfaces
    if (metrics.network) {
        for (const auto& iface : *metrics.network) {
            std::string safeName = sanitizeKey(iface.name);
            file << "NETWORK_" << safeName << "_IN " << iface.totalInOctets << "\n";
            file << "NETWORK_" << safeName << "_OUT " << iface.totalOutOctets << "\n";
        }
    }
    
    // Write disks
    if (metrics.disks) {
        for (const auto& disk : *metrics.disks) {
            std::string safeName = sanitizeKey(disk.deviceName);
            file << "DISK_" << safeName << "_READ " << disk.totalBytesRead << "\n";
            file << "DISK_" << safeName << "_WRITE " << disk.totalBytesWritten << "\n";
        }
    }
    
    file.close();
    return file.good();
}

}  // namespace WinHKMon

