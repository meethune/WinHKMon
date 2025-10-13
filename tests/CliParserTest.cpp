#include "WinHKMonLib/CliParser.h"
#include <gtest/gtest.h>
#include <vector>
#include <string>
#include <cstring>

using namespace WinHKMon;

// Helper function to convert string arguments to argc/argv format
class ArgvHelper {
public:
    ArgvHelper(const std::vector<std::string>& args) {
        argc_ = static_cast<int>(args.size());
        argv_ = new char*[argc_ + 1];
        for (int i = 0; i < argc_; i++) {
            argv_[i] = new char[args[i].size() + 1];
#ifdef _WIN32
            strcpy_s(argv_[i], args[i].size() + 1, args[i].c_str());
#else
            std::strcpy(argv_[i], args[i].c_str());
#endif
        }
        argv_[argc_] = nullptr;
    }

    ~ArgvHelper() {
        for (int i = 0; i < argc_; i++) {
            delete[] argv_[i];
        }
        delete[] argv_;
    }

    int argc() const { return argc_; }
    char** argv() const { return argv_; }

private:
    int argc_;
    char** argv_;
};

// Test metric selection parsing
TEST(CliParserTest, ParsesSingleMetric) {
    ArgvHelper args({"WinHKMon", "CPU"});
    CliOptions opts = parseArguments(args.argc(), args.argv());
    
    EXPECT_TRUE(opts.showCpu);
    EXPECT_FALSE(opts.showMemory);
    EXPECT_FALSE(opts.showDisk);
    EXPECT_FALSE(opts.showNetwork);
    EXPECT_FALSE(opts.showTemp);
}

TEST(CliParserTest, ParsesMultipleMetrics) {
    ArgvHelper args({"WinHKMon", "CPU", "RAM", "NET"});
    CliOptions opts = parseArguments(args.argc(), args.argv());
    
    EXPECT_TRUE(opts.showCpu);
    EXPECT_TRUE(opts.showMemory);
    EXPECT_FALSE(opts.showDisk);
    EXPECT_TRUE(opts.showNetwork);
    EXPECT_FALSE(opts.showTemp);
}

TEST(CliParserTest, MetricsAreCaseInsensitive) {
    ArgvHelper args({"WinHKMon", "cpu", "Ram", "DISK"});
    CliOptions opts = parseArguments(args.argc(), args.argv());
    
    EXPECT_TRUE(opts.showCpu);
    EXPECT_TRUE(opts.showMemory);
    EXPECT_TRUE(opts.showDisk);
}

TEST(CliParserTest, IOAliasForDisk) {
    ArgvHelper args({"WinHKMon", "IO"});
    CliOptions opts = parseArguments(args.argc(), args.argv());
    
    EXPECT_TRUE(opts.showDisk);
}

// Test format flag parsing
TEST(CliParserTest, ParsesFormatJson) {
    ArgvHelper args({"WinHKMon", "CPU", "--format", "json"});
    CliOptions opts = parseArguments(args.argc(), args.argv());
    
    EXPECT_EQ(opts.format, OutputFormat::JSON);
}

TEST(CliParserTest, ParsesFormatCsv) {
    ArgvHelper args({"WinHKMon", "CPU", "-f", "csv"});
    CliOptions opts = parseArguments(args.argc(), args.argv());
    
    EXPECT_EQ(opts.format, OutputFormat::CSV);
}

TEST(CliParserTest, ParsesFormatText) {
    ArgvHelper args({"WinHKMon", "CPU", "--format", "text"});
    CliOptions opts = parseArguments(args.argc(), args.argv());
    
    EXPECT_EQ(opts.format, OutputFormat::TEXT);
}

TEST(CliParserTest, FormatDefaultsToText) {
    ArgvHelper args({"WinHKMon", "CPU"});
    CliOptions opts = parseArguments(args.argc(), args.argv());
    
    EXPECT_EQ(opts.format, OutputFormat::TEXT);
}

// Test single-line mode
TEST(CliParserTest, ParsesLineFlag) {
    ArgvHelper args({"WinHKMon", "CPU", "--line"});
    CliOptions opts = parseArguments(args.argc(), args.argv());
    
    EXPECT_TRUE(opts.singleLine);
}

TEST(CliParserTest, ParsesLineFlagShort) {
    ArgvHelper args({"WinHKMon", "CPU", "-l"});
    CliOptions opts = parseArguments(args.argc(), args.argv());
    
    EXPECT_TRUE(opts.singleLine);
}

TEST(CliParserTest, ParsesLineAsMetric) {
    ArgvHelper args({"WinHKMon", "CPU", "LINE"});
    CliOptions opts = parseArguments(args.argc(), args.argv());
    
    EXPECT_TRUE(opts.singleLine);
}

// Test interval parsing
TEST(CliParserTest, ParsesIntervalInteger) {
    ArgvHelper args({"WinHKMon", "CPU", "--interval", "5"});
    CliOptions opts = parseArguments(args.argc(), args.argv());
    
    EXPECT_DOUBLE_EQ(opts.intervalSeconds, 5.0);
}

TEST(CliParserTest, ParsesIntervalFractional) {
    ArgvHelper args({"WinHKMon", "CPU", "-i", "0.5"});
    CliOptions opts = parseArguments(args.argc(), args.argv());
    
    EXPECT_DOUBLE_EQ(opts.intervalSeconds, 0.5);
}

TEST(CliParserTest, IntervalDefaultsToOne) {
    ArgvHelper args({"WinHKMon", "CPU"});
    CliOptions opts = parseArguments(args.argc(), args.argv());
    
    EXPECT_DOUBLE_EQ(opts.intervalSeconds, 1.0);
}

TEST(CliParserTest, RejectsIntervalTooLow) {
    ArgvHelper args({"WinHKMon", "CPU", "--interval", "0.05"});
    
    EXPECT_THROW({
        parseArguments(args.argc(), args.argv());
    }, std::invalid_argument);
}

TEST(CliParserTest, RejectsIntervalTooHigh) {
    ArgvHelper args({"WinHKMon", "CPU", "--interval", "4000"});
    
    EXPECT_THROW({
        parseArguments(args.argc(), args.argv());
    }, std::invalid_argument);
}

// Test continuous mode
TEST(CliParserTest, ParsesContinuousFlag) {
    ArgvHelper args({"WinHKMon", "CPU", "--continuous"});
    CliOptions opts = parseArguments(args.argc(), args.argv());
    
    EXPECT_TRUE(opts.continuous);
}

TEST(CliParserTest, ParsesContinuousFlagShort) {
    ArgvHelper args({"WinHKMon", "CPU", "-c"});
    CliOptions opts = parseArguments(args.argc(), args.argv());
    
    EXPECT_TRUE(opts.continuous);
}

// Test network interface selection
TEST(CliParserTest, ParsesInterfaceName) {
    ArgvHelper args({"WinHKMon", "NET", "--interface", "Ethernet"});
    CliOptions opts = parseArguments(args.argc(), args.argv());
    
    EXPECT_EQ(opts.networkInterface, "Ethernet");
}

TEST(CliParserTest, ParsesQuotedInterfaceName) {
    ArgvHelper args({"WinHKMon", "NET", "Ethernet 2"});
    CliOptions opts = parseArguments(args.argc(), args.argv());
    
    EXPECT_EQ(opts.networkInterface, "Ethernet 2");
}

// Test network units
TEST(CliParserTest, NetworkUnitsDefaultToBits) {
    ArgvHelper args({"WinHKMon", "NET"});
    CliOptions opts = parseArguments(args.argc(), args.argv());
    
    EXPECT_EQ(opts.networkUnit, NetworkUnit::BITS);
}

TEST(CliParserTest, ParsesNetworkUnitsBits) {
    ArgvHelper args({"WinHKMon", "NET", "--net-units", "bits"});
    CliOptions opts = parseArguments(args.argc(), args.argv());
    
    EXPECT_EQ(opts.networkUnit, NetworkUnit::BITS);
}

TEST(CliParserTest, ParsesNetworkUnitsBytes) {
    ArgvHelper args({"WinHKMon", "NET", "--net-units", "bytes"});
    CliOptions opts = parseArguments(args.argc(), args.argv());
    
    EXPECT_EQ(opts.networkUnit, NetworkUnit::BYTES);
}

// Test help and version
TEST(CliParserTest, ParsesHelpFlag) {
    ArgvHelper args({"WinHKMon", "--help"});
    CliOptions opts = parseArguments(args.argc(), args.argv());
    
    EXPECT_TRUE(opts.showHelp);
}

TEST(CliParserTest, ParsesHelpFlagShort) {
    ArgvHelper args({"WinHKMon", "-h"});
    CliOptions opts = parseArguments(args.argc(), args.argv());
    
    EXPECT_TRUE(opts.showHelp);
}

TEST(CliParserTest, ParsesVersionFlag) {
    ArgvHelper args({"WinHKMon", "--version"});
    CliOptions opts = parseArguments(args.argc(), args.argv());
    
    EXPECT_TRUE(opts.showVersion);
}

TEST(CliParserTest, ParsesVersionFlagShort) {
    ArgvHelper args({"WinHKMon", "-v"});
    CliOptions opts = parseArguments(args.argc(), args.argv());
    
    EXPECT_TRUE(opts.showVersion);
}

// Test invalid arguments
TEST(CliParserTest, RejectsInvalidMetric) {
    ArgvHelper args({"WinHKMon", "INVALID"});
    
    EXPECT_THROW({
        parseArguments(args.argc(), args.argv());
    }, std::invalid_argument);
}

TEST(CliParserTest, RejectsInvalidFormat) {
    ArgvHelper args({"WinHKMon", "CPU", "--format", "xml"});
    
    EXPECT_THROW({
        parseArguments(args.argc(), args.argv());
    }, std::invalid_argument);
}

TEST(CliParserTest, RejectsNoMetrics) {
    ArgvHelper args({"WinHKMon"});
    
    EXPECT_THROW({
        parseArguments(args.argc(), args.argv());
    }, std::invalid_argument);
}

// Test complex combinations
TEST(CliParserTest, ParsesComplexCombination) {
    ArgvHelper args({"WinHKMon", "CPU", "RAM", "NET", "--format", "json", "--continuous", "--interval", "2.5"});
    CliOptions opts = parseArguments(args.argc(), args.argv());
    
    EXPECT_TRUE(opts.showCpu);
    EXPECT_TRUE(opts.showMemory);
    EXPECT_TRUE(opts.showNetwork);
    EXPECT_EQ(opts.format, OutputFormat::JSON);
    EXPECT_TRUE(opts.continuous);
    EXPECT_DOUBLE_EQ(opts.intervalSeconds, 2.5);
}

