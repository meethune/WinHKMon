#pragma once

#include "WinHKMonLib/Types.h"
#include <string>

/**
 * @file OutputFormatter.h
 * @brief Output formatting functions for WinHKMon metrics
 */

namespace WinHKMon {

/**
 * @brief Format metrics as human-readable text
 * 
 * @param metrics System metrics to format
 * @param singleLine If true, output on single line; otherwise multi-line
 * @param options CLI options to determine which metrics to display
 * @return Formatted text string
 */
std::string formatText(const SystemMetrics& metrics, bool singleLine, const CliOptions& options);

/**
 * @brief Format metrics as JSON
 * 
 * @param metrics System metrics to format
 * @param options CLI options to determine which metrics to display
 * @return JSON string
 */
std::string formatJson(const SystemMetrics& metrics, const CliOptions& options);

/**
 * @brief Format metrics as CSV
 * 
 * @param metrics System metrics to format
 * @param includeHeader If true, include CSV header row
 * @param options CLI options to determine which metrics to display
 * @return CSV string
 */
std::string formatCsv(const SystemMetrics& metrics, bool includeHeader, const CliOptions& options);

}  // namespace WinHKMon

