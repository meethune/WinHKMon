#include "WinHKMonLib/CliParser.h"
#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <sstream>

namespace WinHKMon {

namespace {

// Convert string to uppercase for case-insensitive comparison
std::string toUpper(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
    return result;
}

// Check if string starts with dash (flag)
bool isFlag(const std::string& arg) {
    return !arg.empty() && arg[0] == '-';
}

}  // anonymous namespace

std::string generateHelpMessage() {
    return R"(WinHKMon v1.0 - Windows Hardware Monitor

USAGE:
  WinHKMon [METRICS...] [OPTIONS...] [INTERFACE]

METRICS:
  CPU           Monitor CPU usage and frequency
  RAM           Monitor memory (RAM and page file)
  DISK          Monitor disk space (capacity, used, free)
  IO            Monitor disk I/O (read/write rates, busy %)
  NET           Monitor network traffic
  TEMP          Monitor temperature (requires admin)

OPTIONS:
  --format, -f <fmt>     Output format: text, json, csv (default: text)
  --line, -l, LINE       Single-line compact output
  --continuous, -c       Continuous monitoring (until Ctrl+C)
  --interval, -i <sec>   Update interval (default: 1, range: 0.1-3600)
  --net-units <unit>     Network units: bits or bytes (default: bits)
  --interface <name>     Specific network interface
  --help, -h             Show this help
  --version, -v          Show version

EXAMPLES:
  WinHKMon CPU RAM                  # Single sample of CPU and memory
  WinHKMon NET "Ethernet"           # Network stats for specific interface
  WinHKMon CPU RAM -c -i 5          # Continuous monitoring, 5 sec intervals
  WinHKMon CPU TEMP --format json   # JSON output
  WinHKMon CPU RAM LINE             # Single-line output for status bars

For more information: https://github.com/yourorg/WinHKMon
)";
}

std::string generateVersionString() {
    return R"(WinHKMon v1.0.0
Build Date: 2025-10-13
Architecture: x64
)";
}

CliOptions parseArguments(int argc, char* argv[]) {
    CliOptions opts;
    
    // No arguments provided
    if (argc < 2) {
        throw std::invalid_argument("No arguments provided. Use --help for usage information.");
    }
    
    // Parse arguments
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        std::string argUpper = toUpper(arg);
        
        // Help flags (priority 1)
        if (arg == "--help" || arg == "-h") {
            opts.showHelp = true;
            return opts;  // Return immediately, ignore other args
        }
        
        // Version flags (priority 2)
        if (arg == "--version" || arg == "-v") {
            opts.showVersion = true;
            return opts;  // Return immediately, ignore other args
        }
        
        // Metrics (case-insensitive)
        if (argUpper == "CPU") {
            opts.showCpu = true;
        }
        else if (argUpper == "RAM") {
            opts.showMemory = true;
        }
        else if (argUpper == "DISK") {
            opts.showDiskSpace = true;
        }
        else if (argUpper == "IO") {
            opts.showDiskIO = true;
        }
        else if (argUpper == "NET") {
            opts.showNetwork = true;
        }
        else if (argUpper == "TEMP") {
            opts.showTemp = true;
        }
        else if (argUpper == "LINE") {
            opts.singleLine = true;
        }
        
        // Format flags
        else if (arg == "--format" || arg == "-f") {
            if (i + 1 >= argc) {
                throw std::invalid_argument("--format requires an argument (text, json, csv)");
            }
            std::string format = toUpper(argv[++i]);
            if (format == "TEXT") {
                opts.format = OutputFormat::TEXT;
            } else if (format == "JSON") {
                opts.format = OutputFormat::JSON;
            } else if (format == "CSV") {
                opts.format = OutputFormat::CSV;
            } else {
                throw std::invalid_argument("Invalid format '" + std::string(argv[i]) + 
                                          "'. Valid formats: text, json, csv");
            }
        }
        
        // Single-line mode
        else if (arg == "--line" || arg == "-l") {
            opts.singleLine = true;
        }
        
        // Continuous mode
        else if (arg == "--continuous" || arg == "-c") {
            opts.continuous = true;
        }
        
        // Interval
        else if (arg == "--interval" || arg == "-i") {
            if (i + 1 >= argc) {
                throw std::invalid_argument("--interval requires a numeric argument");
            }
            try {
                double interval = std::stod(argv[++i]);
                if (interval < 0.1 || interval > 3600.0) {
                    throw std::invalid_argument(
                        "Interval must be between 0.1 and 3600 seconds. Got: " + 
                        std::to_string(interval));
                }
                opts.intervalSeconds = interval;
            } catch (const std::invalid_argument&) {
                throw std::invalid_argument("Invalid interval value: " + std::string(argv[i]));
            }
        }
        
        // Network interface
        else if (arg == "--interface") {
            if (i + 1 >= argc) {
                throw std::invalid_argument("--interface requires an interface name");
            }
            opts.networkInterface = argv[++i];
        }
        
        // Network units
        else if (arg == "--net-units") {
            if (i + 1 >= argc) {
                throw std::invalid_argument("--net-units requires an argument (bits, bytes)");
            }
            std::string units = toUpper(argv[++i]);
            if (units == "BITS") {
                opts.networkUnit = NetworkUnit::BITS;
            } else if (units == "BYTES") {
                opts.networkUnit = NetworkUnit::BYTES;
            } else {
                throw std::invalid_argument("Invalid network unit '" + std::string(argv[i]) + 
                                          "'. Valid units: bits, bytes");
            }
        }
        
        // Interface name without flag (only if NET already specified or looks like interface)
        else if (!isFlag(arg) && opts.showNetwork) {
            // Interface name for already-enabled network monitoring
            opts.networkInterface = arg;
        }
        
        // Unknown argument
        else {
            throw std::invalid_argument("Invalid metric or flag '" + arg + 
                                      "'. Use --help for usage information.");
        }
    }
    
    // Validation: At least one metric must be selected (unless help/version)
    if (!opts.showHelp && !opts.showVersion) {
        if (!opts.showCpu && !opts.showMemory && !opts.showDiskSpace && !opts.showDiskIO &&
            !opts.showNetwork && !opts.showTemp) {
            throw std::invalid_argument(
                "At least one metric must be specified (CPU, RAM, DISK, IO, NET, TEMP). "
                "Use --help for usage information.");
        }
    }
    
    return opts;
}

}  // namespace WinHKMon

