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
 * @return Formatted text string
 */
std::string formatText(const SystemMetrics& metrics, bool singleLine);

/**
 * @brief Format metrics as JSON
 * 
 * @param metrics System metrics to format
 * @return JSON string
 */
std::string formatJson(const SystemMetrics& metrics);

/**
 * @brief Format metrics as CSV
 * 
 * @param metrics System metrics to format
 * @param includeHeader If true, include CSV header row
 * @return CSV string
 */
std::string formatCsv(const SystemMetrics& metrics, bool includeHeader);

}  // namespace WinHKMon

