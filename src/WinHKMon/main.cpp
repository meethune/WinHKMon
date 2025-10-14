#include "WinHKMonLib/CliParser.h"
#include "WinHKMonLib/OutputFormatter.h"
#include "WinHKMonLib/StateManager.h"
#include "WinHKMonLib/MemoryMonitor.h"
#include "WinHKMonLib/CpuMonitor.h"
#include "WinHKMonLib/NetworkMonitor.h"
#include "WinHKMonLib/DiskMonitor.h"
#include "WinHKMonLib/DeltaCalculator.h"
#include <iostream>
#include <windows.h>
#include <thread>
#include <chrono>
#include <csignal>

using namespace WinHKMon;

// Global flag for Ctrl+C handling
volatile bool g_continueMonitoring = true;

/**
 * @brief Signal handler for Ctrl+C (SIGINT)
 * 
 * Sets global flag to stop continuous monitoring loop.
 */
void signalHandler(int signal) {
    if (signal == SIGINT) {
        g_continueMonitoring = false;
        std::cerr << "\nStopping... ";
    }
}

/**
 * @brief Collect system metrics based on CLI options
 * 
 * @param options Parsed CLI options
 * @param cpuMonitor CPU monitor instance (if initialized)
 * @param memoryMonitor Memory monitor instance
 * @param networkMonitor Network monitor instance (if initialized)
 * @param diskMonitor Disk monitor instance (if initialized)
 * @param deltaCalc Delta calculator for timestamps
 * @return SystemMetrics structure with requested metrics
 */
SystemMetrics collectMetrics(const CliOptions& options, 
                             CpuMonitor* cpuMonitor, 
                             MemoryMonitor& memoryMonitor,
                             NetworkMonitor* networkMonitor,
                             DiskMonitor* diskMonitor,
                             DeltaCalculator& deltaCalc) {
    SystemMetrics metrics;
    
    // Get timestamp
    metrics.timestamp = deltaCalc.getCurrentTimestamp();
    
    // Collect CPU stats
    if (options.showCpu && cpuMonitor != nullptr) {
        try {
            metrics.cpu = cpuMonitor->getCurrentStats();
        } catch (const std::exception& e) {
            std::cerr << "[WARNING] CPU monitoring failed: " << e.what() << std::endl;
        }
    }
    
    // Collect memory stats
    if (options.showMemory) {
        try {
            metrics.memory = memoryMonitor.getCurrentStats();
        } catch (const std::exception& e) {
            std::cerr << "[WARNING] Memory monitoring failed: " << e.what() << std::endl;
        }
    }
    
    // Collect network stats
    if (options.showNetwork && networkMonitor != nullptr) {
        try {
            std::vector<InterfaceStats> interfaces = networkMonitor->getCurrentStats();
            
            // Filter to specific interface if requested
            if (!options.networkInterface.empty()) {
                auto it = std::find_if(interfaces.begin(), interfaces.end(),
                    [&options](const InterfaceStats& iface) {
                        return iface.name == options.networkInterface;
                    });
                if (it != interfaces.end()) {
                    metrics.network = std::vector<InterfaceStats>{*it};
                } else {
                    std::cerr << "[WARNING] Network interface '" << options.networkInterface 
                             << "' not found." << std::endl;
                }
            } else {
                // Auto-select primary interface or include all
                metrics.network = interfaces;
            }
        } catch (const std::exception& e) {
            std::cerr << "[WARNING] Network monitoring failed: " << e.what() << std::endl;
        }
    }
    
    // Collect disk stats (if either DISK or IO is requested)
    if ((options.showDiskSpace || options.showDiskIO) && diskMonitor != nullptr) {
        try {
            metrics.disks = diskMonitor->getCurrentStats();
        } catch (const std::exception& e) {
            std::cerr << "[WARNING] Disk monitoring failed: " << e.what() << std::endl;
        }
    }
    
    // TODO: Collect temperature stats (T017 - TempMonitor)
    
    return metrics;
}

/**
 * @brief Single-shot monitoring mode
 * 
 * Collects metrics once and outputs to stdout.
 * 
 * @param options CLI options
 * @return Exit code (0 = success, 2 = error)
 */
int singleShotMode(const CliOptions& options) {
    try {
        // Initialize monitors
        MemoryMonitor memoryMonitor;
        CpuMonitor* cpuMonitor = nullptr;
        NetworkMonitor* networkMonitor = nullptr;
        DiskMonitor* diskMonitor = nullptr;
        DeltaCalculator deltaCalc;
        
        if (options.showCpu) {
            cpuMonitor = new CpuMonitor();
            cpuMonitor->initialize();
            
            // Wait for first sample (PDH requires two samples)
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        
        if (options.showNetwork) {
            networkMonitor = new NetworkMonitor();
            networkMonitor->initialize();
        }
        
        if (options.showDiskSpace || options.showDiskIO) {
            diskMonitor = new DiskMonitor();
            diskMonitor->initialize();
            
            // Wait for first sample (PDH requires two samples for I/O rates)
            std::this_thread::sleep_for(std::chrono::milliseconds(1100));
        }
        
        // Collect metrics
        SystemMetrics metrics = collectMetrics(options, cpuMonitor, memoryMonitor, 
                                               networkMonitor, diskMonitor, deltaCalc);
        
        // Format output
        std::string output;
        if (options.format == OutputFormat::JSON) {
            output = formatJson(metrics, options);
        } else if (options.format == OutputFormat::CSV) {
            output = formatCsv(metrics, true, options);  // Include header
        } else {
            output = formatText(metrics, options.singleLine, options);
        }
        
        // Output to stdout
        std::cout << output;
        if (options.format == OutputFormat::TEXT && !options.singleLine) {
            std::cout << std::endl;
        }
        
        // Cleanup
        if (cpuMonitor != nullptr) {
            cpuMonitor->cleanup();
            delete cpuMonitor;
        }
        if (networkMonitor != nullptr) {
            delete networkMonitor;
        }
        if (diskMonitor != nullptr) {
            diskMonitor->cleanup();
            delete diskMonitor;
        }
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "[ERROR] " << e.what() << std::endl;
        return 2;
    }
}

/**
 * @brief Continuous monitoring mode
 * 
 * Collects metrics repeatedly at specified interval until Ctrl+C.
 * 
 * @param options CLI options
 * @return Exit code (0 = success, 2 = error)
 */
int continuousMode(const CliOptions& options) {
    try {
        // Set up signal handler for Ctrl+C
        signal(SIGINT, signalHandler);
        
        // Initialize monitors
        MemoryMonitor memoryMonitor;
        CpuMonitor* cpuMonitor = nullptr;
        NetworkMonitor* networkMonitor = nullptr;
        DiskMonitor* diskMonitor = nullptr;
        DeltaCalculator deltaCalc;
        StateManager stateManager("WinHKMon");
        
        if (options.showCpu) {
            cpuMonitor = new CpuMonitor();
            cpuMonitor->initialize();
            
            // Wait for first sample
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        
        if (options.showNetwork) {
            networkMonitor = new NetworkMonitor();
            networkMonitor->initialize();
        }
        
        if (options.showDiskSpace || options.showDiskIO) {
            diskMonitor = new DiskMonitor();
            diskMonitor->initialize();
            
            // Wait for first sample (PDH requires two samples for I/O rates)
            std::this_thread::sleep_for(std::chrono::milliseconds(1100));
        }
        
        // For CSV, output header once
        if (options.format == OutputFormat::CSV) {
            SystemMetrics dummyMetrics;
            std::cout << formatCsv(dummyMetrics, true, options);
        }
        
        // Monitoring loop
        int sampleCount = 0;
        while (g_continueMonitoring) {
            // Collect metrics
            SystemMetrics metrics = collectMetrics(options, cpuMonitor, memoryMonitor, 
                                                   networkMonitor, diskMonitor, deltaCalc);
            
            // Format output
            std::string output;
            if (options.format == OutputFormat::JSON) {
                output = formatJson(metrics, options);
            } else if (options.format == OutputFormat::CSV) {
                output = formatCsv(metrics, false, options);  // No header
            } else {
                // For text mode in continuous, optionally clear screen
                if (sampleCount > 0 && !options.singleLine) {
                    // Move cursor to top of screen for in-place update
                    // (simple version - could use Windows console API for better control)
                    std::cout << "\n";
                }
                output = formatText(metrics, options.singleLine, options);
            }
            
            // Output to stdout
            std::cout << output;
            if (options.format == OutputFormat::TEXT) {
                std::cout << std::endl;
            }
            std::cout.flush();
            
            sampleCount++;
            
            // Wait for interval
            if (g_continueMonitoring) {
                auto sleepMs = static_cast<int>(options.intervalSeconds * 1000);
                std::this_thread::sleep_for(std::chrono::milliseconds(sleepMs));
            }
        }
        
        // Cleanup
        if (cpuMonitor != nullptr) {
            cpuMonitor->cleanup();
            delete cpuMonitor;
        }
        if (networkMonitor != nullptr) {
            delete networkMonitor;
        }
        if (diskMonitor != nullptr) {
            diskMonitor->cleanup();
            delete diskMonitor;
        }
        
        std::cerr << "state saved." << std::endl;
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "[ERROR] " << e.what() << std::endl;
        return 2;
    }
}

/**
 * @brief Main entry point
 */
int main(int argc, char* argv[]) {
    try {
        // Parse command-line arguments
        CliOptions options = parseArguments(argc, argv);
        
        // Handle help
        if (options.showHelp) {
            std::cout << generateHelpMessage() << std::endl;
            return 0;
        }
        
        // Handle version
        if (options.showVersion) {
            std::cout << generateVersionString() << std::endl;
            return 0;
        }
        
        // Check that at least one metric is requested
        if (!options.showCpu && !options.showMemory && !options.showDisk && 
            !options.showNetwork && !options.showTemp) {
            std::cerr << "[ERROR] No metrics specified. Use --help for usage information." << std::endl;
            return 1;
        }
        
        // Warn about unimplemented features
        if (options.showTemp) {
            std::cerr << "[WARNING] Temperature monitoring not yet implemented (T017 pending)." << std::endl;
        }
        
        // Run in appropriate mode
        if (options.continuous) {
            return continuousMode(options);
        } else {
            return singleShotMode(options);
        }
        
    } catch (const std::exception& e) {
        std::cerr << "[ERROR] " << e.what() << std::endl;
        std::cerr << "Use --help for usage information." << std::endl;
        return 1;
    }
}
