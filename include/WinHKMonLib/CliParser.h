#pragma once

#include "WinHKMonLib/Types.h"
#include <string>

/**
 * @file CliParser.h
 * @brief Command-line argument parser for WinHKMon
 */

namespace WinHKMon {

/**
 * @brief Parse command-line arguments into CliOptions structure
 * 
 * @param argc Argument count
 * @param argv Argument values
 * @return Parsed CLI options
 * @throws std::invalid_argument if arguments are invalid
 */
CliOptions parseArguments(int argc, char* argv[]);

/**
 * @brief Generate help message
 * @return Help text string
 */
std::string generateHelpMessage();

/**
 * @brief Generate version information
 * @return Version string
 */
std::string generateVersionString();

}  // namespace WinHKMon

