#include "log_generator.h"
#include "log_parser.h"
#include <iostream>
#include <iomanip>
#include <string>
#include <chrono>
#include <thread>

void printStats(const GenerationStats& genStats, const ParsingStats& parseStats) {
    std::cout << "\n=== PERFORMANCE STATISTICS ===\n";
    
    // Log generation stats
    std::cout << "Log Generation:\n";
    std::cout << "  - Logs generated: " << genStats.logsGenerated << "\n";
    std::cout << "  - Threads used: " << genStats.threadsUsed << "\n";
    std::cout << "  - Time: " << std::fixed << std::setprecision(2) << genStats.elapsedTimeMs << " ms\n";
    std::cout << "  - Throughput: " << std::fixed << std::setprecision(2) 
              << (genStats.logsGenerated * 1000.0 / genStats.elapsedTimeMs) << " logs/second\n";
    
    // Log parsing stats
    std::cout << "\nLog Parsing:\n";
    std::cout << "  - Lines parsed: " << parseStats.linesParsed << "\n";
    std::cout << "  - Threads used: " << parseStats.threadsUsed << "\n";
    std::cout << "  - Time: " << std::fixed << std::setprecision(2) << parseStats.elapsedTimeMs << " ms\n";
    std::cout << "  - Throughput: " << std::fixed << std::setprecision(2) 
              << (parseStats.linesParsed * 1000.0 / parseStats.elapsedTimeMs) << " lines/second\n";
    
    // Log level distribution
    std::cout << "\nLog Level Distribution:\n";
    for (const auto& [level, count] : parseStats.logLevelCounts) {
        std::cout << "  - " << level << ": " << count << " logs (" 
                  << std::fixed << std::setprecision(1) 
                  << (count * 100.0 / parseStats.linesParsed) << "%)\n";
    }
    
    std::cout << "==============================\n";
}

int main(int argc, char* argv[]) {
    // Default parameters
    std::string filename = "../logs/sample.log";
    int logCount = 100000;
    int threadCount = 0; // 0 means auto-detect
    
    // Parse command line arguments if provided
    if (argc > 1) logCount = std::stoi(argv[1]);
    if (argc > 2) threadCount = std::stoi(argv[2]);
    if (argc > 3) filename = argv[3];
    
    // Determine optimal thread count
    if (threadCount <= 0) {
        threadCount = getOptimalThreadCount();
    }
    
    std::cout << "=== LogFast High-Performance Log Analyzer ===\n";
    std::cout << "Generating " << logCount << " logs using " << threadCount << " threads...\n";
    
    // Generate logs with multithreading
    auto genStats = generateLogs(filename, logCount, threadCount);
    
    std::cout << "Log generation completed in " << std::fixed << std::setprecision(2) 
              << genStats.elapsedTimeMs << " ms\n";
    
    // Brief pause to allow file system to sync
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    std::cout << "\nParsing logs from " << filename << " using " << threadCount << " threads...\n";
    
    // Parse logs with multithreading
    auto parseStats = parseLogs(filename, threadCount);
    
    std::cout << "Log parsing completed in " << std::fixed << std::setprecision(2) 
              << parseStats.elapsedTimeMs << " ms\n";
    
    // Print detailed performance statistics
    printStats(genStats, parseStats);
    
    return 0;
}
