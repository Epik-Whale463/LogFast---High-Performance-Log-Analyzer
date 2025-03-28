#ifndef LOG_PARSER_H
#define LOG_PARSER_H

#include <string>
#include <vector>
#include <unordered_map>

// Struct to hold parsing statistics
struct ParsingStats {
    double elapsedTimeMs;
    int linesParsed;
    int threadsUsed;
    std::unordered_map<std::string, int> logLevelCounts;
};

// Enhanced log parsing with multithreading
ParsingStats parseLogs(const std::string &filename, int numThreads = 0);

// Utility function to get optimal thread count
int getOptimalThreadCount();

#endif