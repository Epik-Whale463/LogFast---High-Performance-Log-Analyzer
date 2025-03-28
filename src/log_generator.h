#ifndef LOG_GENERATOR_H
#define LOG_GENERATOR_H

#include <string>
#include <future>
#include <vector>

// Struct to hold generation statistics
struct GenerationStats {
    double elapsedTimeMs;
    int logsGenerated;
    int threadsUsed;
};

// Enhanced log generation with multithreading and statistics
GenerationStats generateLogs(const std::string &filename, int count, int numThreads = 0);

// Get optimal thread count based on hardware
int getOptimalThreadCount();

#endif
