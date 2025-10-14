/**
 * @file TempMonitor.cpp
 * @brief Temperature monitoring implementation using LibreHardwareMonitor
 * 
 * IMPORTANT: This file requires C++/CLI compilation with .NET Desktop Development workload.
 * 
 * Compilation Requirements:
 * - Visual Studio 2022 with .NET Desktop Development
 * - LibreHardwareMonitorLib NuGet package or DLL
 * - Compile with /clr flag: set_source_files_properties(...TempMonitor.cpp PROPERTIES COMPILE_FLAGS "/clr")
 * 
 * Dependencies:
 * - LibreHardwareMonitorLib.dll (MPL 2.0)
 * - .NET Runtime 4.7.2+ or .NET 6+
 * - WinRing0 kernel driver (bundled with LHM)
 * 
 * Security Considerations:
 * - Requires administrator privileges to load kernel driver
 * - Never auto-elevate privileges
 * - Gracefully degrade when admin unavailable
 * - Validate all sensor readings
 */

#include "TempMonitor.h"
#include <Windows.h>
#include <sddl.h>
#include <algorithm>
#include <vector>

// NOTE: When compiling on Windows with Visual Studio, uncomment these C++/CLI includes:
// #using <LibreHardwareMonitorLib.dll>
// using namespace LibreHardwareMonitor::Hardware;
// using namespace System;
// using namespace System::Collections::Generic;

namespace WinHKMon {

// ============================================================================
// Admin Privilege Check
// ============================================================================

bool AdminPrivileges::IsRunningAsAdmin() {
    BOOL isAdmin = FALSE;
    PSID administratorsGroup = NULL;
    
    // Create SID for BUILTIN\Administrators group
    SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;
    
    if (AllocateAndInitializeSid(
        &ntAuthority,
        2,
        SECURITY_BUILTIN_DOMAIN_RID,
        SECURITY_LOCAL_RID_ADMINS,
        0, 0, 0, 0, 0, 0,
        &administratorsGroup)) {
        
        // Check if current token is member of Administrators group
        if (!CheckTokenMembership(NULL, administratorsGroup, &isAdmin)) {
            isAdmin = FALSE;
        }
        
        FreeSid(administratorsGroup);
    }
    
    return isAdmin == TRUE;
}

// ============================================================================
// TempMonitor Implementation (PIMPL Pattern)
// ============================================================================

/**
 * @brief Private implementation class for TempMonitor
 * 
 * This class will contain managed C++ code when compiled with /clr.
 * Uses PIMPL pattern to hide C++/CLI types from header.
 */
class TempMonitor::Impl {
public:
    Impl() : computer_(nullptr), hasCpuSensors_(false) {}
    
    ~Impl() {
        cleanup();
    }
    
    TempMonitor::InitResult initialize() {
        // TODO: When compiling on Windows, implement:
        // 1. Check if running as admin
        // 2. Try to create Computer^ instance from LibreHardwareMonitor
        // 3. Enable CPU and GPU sensors
        // 4. Call computer->Open()
        // 5. Check if any sensors available
        // 6. Return appropriate InitResult
        
        // STUB IMPLEMENTATION (will be replaced):
        return TempMonitor::InitResult::LIBRARY_MISSING;
        
        /* Windows C++/CLI Implementation Template:
        
        try {
            // Create managed Computer object
            computer_ = gcnew Computer();
            computer_->IsCpuEnabled = true;
            computer_->IsGpuEnabled = true;  // Optional
            
            // Open hardware monitoring
            computer_->Open();
            
            // Check if CPU sensors available
            bool foundCpuSensors = false;
            for each (IHardware^ hardware in computer_->Hardware) {
                if (hardware->HardwareType == HardwareType::Cpu) {
                    hardware->Update();
                    for each (ISensor^ sensor in hardware->Sensors) {
                        if (sensor->SensorType == SensorType::Temperature) {
                            foundCpuSensors = true;
                            break;
                        }
                    }
                }
            }
            
            if (!foundCpuSensors) {
                return TempMonitor::InitResult::NO_SENSORS;
            }
            
            hasCpuSensors_ = true;
            return TempMonitor::InitResult::SUCCESS;
        }
        catch (System::DllNotFoundException^) {
            return TempMonitor::InitResult::LIBRARY_MISSING;
        }
        catch (System::UnauthorizedAccessException^) {
            return TempMonitor::InitResult::DRIVER_FAILED;
        }
        catch (...) {
            return TempMonitor::InitResult::DRIVER_FAILED;
        }
        */
    }
    
    std::optional<TempStats> getCurrentStats() {
        // TODO: When compiling on Windows, implement:
        // 1. Update all hardware sensors
        // 2. Iterate through CPU sensors
        // 3. Extract temperature values
        // 4. Calculate min/max/avg
        // 5. Populate TempStats structure
        // 6. Return stats
        
        // STUB IMPLEMENTATION (will be replaced):
        return std::nullopt;
        
        /* Windows C++/CLI Implementation Template:
        
        if (computer_ == nullptr || !hasCpuSensors_) {
            return std::nullopt;
        }
        
        try {
            TempStats stats;
            std::vector<int> cpuTemps;
            
            // Update and collect temperature data
            for each (IHardware^ hardware in computer_->Hardware) {
                hardware->Update();
                
                if (hardware->HardwareType == HardwareType::Cpu) {
                    for each (ISensor^ sensor in hardware->Sensors) {
                        if (sensor->SensorType == SensorType::Temperature) {
                            if (sensor->Value.HasValue) {
                                int temp = static_cast<int>(sensor->Value.Value);
                                
                                // Convert managed string to std::string
                                String^ managedName = sensor->Name;
                                std::string sensorName;
                                for (int i = 0; i < managedName->Length; i++) {
                                    sensorName += static_cast<char>(managedName[i]);
                                }
                                
                                TempStats::SensorReading reading;
                                reading.name = sensorName;
                                reading.tempCelsius = temp;
                                reading.hardwareType = "CPU";
                                
                                stats.cpuTemps.push_back(reading);
                                cpuTemps.push_back(temp);
                            }
                        }
                    }
                }
            }
            
            // Calculate statistics
            if (!cpuTemps.empty()) {
                stats.maxCpuTempCelsius = *std::max_element(cpuTemps.begin(), cpuTemps.end());
                return stats;
            }
            
            return std::nullopt;
        }
        catch (...) {
            return std::nullopt;
        }
        */
    }
    
    void cleanup() {
        // TODO: When compiling on Windows, implement:
        // 1. Close computer object
        // 2. Release managed resources
        
        // STUB IMPLEMENTATION:
        computer_ = nullptr;
        hasCpuSensors_ = false;
        
        /* Windows C++/CLI Implementation Template:
        
        if (computer_ != nullptr) {
            try {
                computer_->Close();
            }
            catch (...) {
                // Ignore errors during cleanup
            }
            computer_ = nullptr;
        }
        hasCpuSensors_ = false;
        */
    }

private:
    // Managed pointer (gcroot) - requires C++/CLI
    void* computer_;  // In C++/CLI: gcroot<Computer^> computer_;
    bool hasCpuSensors_;
};

// ============================================================================
// TempMonitor Public Interface
// ============================================================================

TempMonitor::TempMonitor()
    : isInitialized_(false)
    , pImpl_(new Impl())
{
}

TempMonitor::~TempMonitor() {
    cleanup();
    delete pImpl_;
}

TempMonitor::InitResult TempMonitor::initialize() {
    // Check admin privileges first (always required)
    if (!AdminPrivileges::IsRunningAsAdmin()) {
        return InitResult::NO_ADMIN;
    }
    
    // Attempt to initialize LibreHardwareMonitor
    InitResult result = pImpl_->initialize();
    
    if (result == InitResult::SUCCESS) {
        isInitialized_ = true;
    }
    
    return result;
}

std::optional<TempStats> TempMonitor::getCurrentStats() {
    if (!isInitialized_) {
        return std::nullopt;
    }
    
    return pImpl_->getCurrentStats();
}

void TempMonitor::cleanup() {
    if (isInitialized_) {
        pImpl_->cleanup();
        isInitialized_ = false;
    }
}

std::string TempMonitor::getInitResultMessage(InitResult result) {
    switch (result) {
        case InitResult::SUCCESS:
            return "Temperature monitoring initialized successfully";
        
        case InitResult::NO_ADMIN:
            return "Administrator privileges required for temperature monitoring.\n"
                   "Right-click WinHKMon and select 'Run as administrator'";
        
        case InitResult::NO_SENSORS:
            return "No temperature sensors detected on this system.\n"
                   "This is common in virtual machines or older hardware";
        
        case InitResult::DRIVER_FAILED:
            return "Failed to load hardware monitoring drivers.\n"
                   "Ensure WinRing0.sys driver is available and signed";
        
        case InitResult::LIBRARY_MISSING:
            return "LibreHardwareMonitorLib.dll not found.\n"
                   "Ensure the library is in the same directory as WinHKMon.exe";
        
        default:
            return "Unknown initialization error";
    }
}

} // namespace WinHKMon

