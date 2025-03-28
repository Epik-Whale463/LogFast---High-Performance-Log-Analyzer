#include "log_generator.h"
#include <iostream>
#include <fstream>
#include <ctime>
#include <thread>
#include <vector>
#include <mutex>

std::mutex logMutex;

void logWorker(const std::string &filename, int start, int end) {
    std::ofstream logFile(filename, std::ios::app); 
    if (!logFile) {
        std::cerr << "Error: Unable to open log file\n";
        return;
    }

    std::string levels[] = {"INFO", "WARNING", "ERROR"};

    for (int i = start; i < end; i++) {
        time_t now = time(0);
        char *dt = ctime(&now);
        dt[strlen(dt) - 1] = '\0'; 

        std::lock_guard<std::mutex> guard(logMutex);
        logFile << dt << " [" << levels[i % 3] << "] Sample log entry " << i << "\n";
    }

    logFile.close();
}

void generateLogs(const std::string &filename, int count, int numThreads) {
    std::ofstream logFile(filename, std::ios::trunc);
    logFile.close(); 

    std::vector<std::thread> threads;
    int logsPerThread = count / numThreads;

    for (int i = 0; i < numThreads; i++) {
        int start = i * logsPerThread;
        int end = (i == numThreads - 1) ? count : start + logsPerThread;
        threads.emplace_back(logWorker, filename, start, end);
    }

    for (auto &t : threads) {
        t.join();
    }
}
