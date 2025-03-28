#include "log_generator.h"
#include <iostream>

int main() {
    std::string filename = "logs.txt";
    int logCount = 1000;
    int numThreads = 4; 

    std::cout << "Generating " << logCount << " logs using " << numThreads << " threads...\n";
    generateLogs(filename, logCount, numThreads);

    std::cout << "Log generation complete! Check logs.txt\n";
    return 0;
}
