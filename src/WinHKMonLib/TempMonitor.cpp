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

#pragma managed(push, off)
#include "TempMonitor.h"
#include <Windows.h>
#include <sddl.h>
#include <algorithm>
#include <vector>
#pragma managed(pop)

// C++/CLI includes for LibreHardwareMonitor integration
#using <System.dll>
#using <mscorlib.dll>
using namespace System;
using namespace System::Collections::Generic;

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

// Forward declare managed type
#pragma managed(push, on)
using namespace LibreHardwareMonitor::Hardware;

// Helper function to convert managed String^ to std::string
static std::string ManagedToNative(String^ str) {
    if (str == nullptr || str->Length == 0) {
        return "";
    }
    
    std::string result;
    result.reserve(str->Length);
    for (int i = 0; i < str->Length; i++) {
        wchar_t wc = str[i];
        if (wc < 128) {
            result += static_cast<char>(wc);
        } else {
            result += '?';  // Simple fallback for non-ASCII
        }
    }
    return result;
}

/**
 * @brief Private implementation class for TempMonitor
 * 
 * This class contains managed C++ code compiled with /clr.
 * Uses PIMPL pattern to hide C++/CLI types from header.
 */
class TempMonitor::Impl {
public:
    Impl() : computer_(nullptr), hasCpuSensors_(false) {}
    
    ~Impl() {
        cleanup();
    }
    
    TempMonitor::InitResult initialize() {
        try {
            // Create managed Computer object
            computer_ = gcnew Computer();
            computer_->IsCpuEnabled = true;
            computer_->IsGpuEnabled = true;  // Optional for future FR-5.4
            computer_->IsMotherboardEnabled = true;  // Optional for future FR-5.5
            
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
                    if (foundCpuSensors) break;
                }
            }
            
            if (!foundCpuSensors) {
                cleanup();
                return TempMonitor::InitResult::NO_SENSORS;
            }
            
            hasCpuSensors_ = true;
            return TempMonitor::InitResult::SUCCESS;
        }
        catch (System::IO::FileNotFoundException^) {
            return TempMonitor::InitResult::LIBRARY_MISSING;
        }
        catch (System::DllNotFoundException^) {
            return TempMonitor::InitResult::LIBRARY_MISSING;
        }
        catch (System::UnauthorizedAccessException^) {
            return TempMonitor::InitResult::DRIVER_FAILED;
        }
        catch (System::Exception^ e) {
            // Log the exception message for debugging
            System::Diagnostics::Debug::WriteLine("TempMonitor initialization failed: " + e->Message);
            return TempMonitor::InitResult::DRIVER_FAILED;
        }
        catch (...) {
            return TempMonitor::InitResult::DRIVER_FAILED;
        }
    }
    
    std::optional<TempStats> getCurrentStats() {
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
                        if (sensor->SensorType == SensorType::Temperature && sensor->Value.HasValue) {
                            int temp = static_cast<int>(sensor->Value.Value);
                            
                            // Validate temperature is in reasonable range
                            if (temp < 0 || temp > 150) {
                                continue;  // Skip invalid readings
                            }
                            
                            TempStats::SensorReading reading;
                            reading.name = ManagedToNative(sensor->Name);
                            reading.tempCelsius = temp;
                            reading.hardwareType = "CPU";
                            
                            stats.cpuTemps.push_back(reading);
                            cpuTemps.push_back(temp);
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
        catch (System::Exception^) {
            return std::nullopt;
        }
        catch (...) {
            return std::nullopt;
        }
    }
    
    void cleanup() {
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
    }

private:
    Computer^ computer_;
    bool hasCpuSensors_;
};

#pragma managed(pop)

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

