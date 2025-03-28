#include "log_parser.h"
#include "log_generator.h"
#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <vector>
#include <mutex>
#include <atomic>
#include <chrono>
#include <regex>
#include <queue>
#include <condition_variable>
#include <future>
#include <algorithm>

// Thread-safe queue implementation
template<typename T>
class ThreadSafeQueue {
private:
    std::queue<T> queue;
    std::mutex mutex;
    std::condition_variable cv;
    bool done = false;

public:
    void push(T item) {
        std::unique_lock<std::mutex> lock(mutex);
        queue.push(std::move(item));
        lock.unlock();
        cv.notify_one();
    }

    bool pop(T& item) {
        std::unique_lock<std::mutex> lock(mutex);
        cv.wait(lock, [this] { return !queue.empty() || done; });
        
        if (queue.empty() && done) {
            return false;
        }
        
        item = std::move(queue.front());
        queue.pop();
        return true;
    }

    void finish() {
        std::unique_lock<std::mutex> lock(mutex);
        done = true;
        lock.unlock();
        cv.notify_all();
    }

    size_t size() {
        std::lock_guard<std::mutex> lock(mutex);
        return queue.size();
    }
};

// Thread-safe map for counting log levels
class LogLevelCounter {
private:
    std::unordered_map<std::string, int> counts;
    mutable std::mutex mutex;

public:
    void increment(const std::string& level) {
        std::lock_guard<std::mutex> lock(mutex);
        counts[level]++;
    }

    std::unordered_map<std::string, int> getCounts() const {
        std::lock_guard<std::mutex> lock(mutex);
        return counts;
    }
};

// Fast log line parser using regex
class LogLineParser {
private:
    std::regex pattern;

public:
    LogLineParser() : pattern(R"(.*\[(INFO|WARNING|ERROR|DEBUG)\].*?)") {}

    std::string extractLogLevel(const std::string& line) {
        std::smatch matches;
        if (std::regex_search(line, matches, pattern) && matches.size() > 1) {
            return matches[1].str();
        }
        return "UNKNOWN";
    }
};

// Function to read file in chunks and push to queue
void fileReader(const std::string& filename, ThreadSafeQueue<std::vector<std::string>>& linesQueue, 
               std::atomic<bool>& readerDone, int chunkSize) {
    std::ifstream file(filename);
    if (!file) {
        std::cerr << "Error opening file: " << filename << std::endl;
        readerDone = true;
        linesQueue.finish();
        return;
    }

    std::vector<std::string> chunk;
    chunk.reserve(chunkSize);
    
    std::string line;
    while (std::getline(file, line)) {
        chunk.push_back(line);
        
        if (chunk.size() >= chunkSize) {
            linesQueue.push(std::move(chunk));
            chunk = std::vector<std::string>();
            chunk.reserve(chunkSize);
        }
    }
    
    // Push any remaining lines
    if (!chunk.empty()) {
        linesQueue.push(std::move(chunk));
    }
    
    file.close();
    readerDone = true;
    linesQueue.finish();
}

// Worker function to process chunks of lines
void processChunks(ThreadSafeQueue<std::vector<std::string>>& linesQueue, 
                  LogLevelCounter& counter,
                  std::atomic<int>& linesParsed) {
    LogLineParser parser;
    std::vector<std::string> chunk;
    
    while (linesQueue.pop(chunk)) {
        for (const auto& line : chunk) {
            std::string logLevel = parser.extractLogLevel(line);
            counter.increment(logLevel);
            linesParsed++;
            
            // Output line to console (can be disabled for performance)
            std::cout << line << std::endl;
        }
    }
}

// Main parsing function
ParsingStats parseLogs(const std::string &filename, int numThreads) {
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // Use optimal thread count if not specified
    if (numThreads <= 0) {
        numThreads = getOptimalThreadCount();
    }
    
    // Cap threads to a reasonable maximum
    numThreads = std::min(numThreads, 32);
    
    // Initialize shared resources
    ThreadSafeQueue<std::vector<std::string>> linesQueue;
    LogLevelCounter logLevelCounter;
    std::atomic<int> linesParsed(0);
    std::atomic<bool> readerDone(false);
    
    // Determine chunk size based on thread count
    const int chunkSize = 1000;
    
    // Start file reader thread
    std::thread reader(fileReader, filename, std::ref(linesQueue), std::ref(readerDone), chunkSize);
    
    // Start worker threads
    std::vector<std::thread> workers;
    for (int i = 0; i < numThreads; ++i) {
        workers.emplace_back(processChunks, 
                           std::ref(linesQueue), 
                           std::ref(logLevelCounter),
                           std::ref(linesParsed));
    }
    
    // Wait for reader to finish
    if (reader.joinable()) {
        reader.join();
    }
    
    // Wait for all workers to finish
    for (auto& worker : workers) {
        if (worker.joinable()) {
            worker.join();
        }
    }
    
    // Calculate elapsed time
    auto endTime = std::chrono::high_resolution_clock::now();
    double elapsedMs = std::chrono::duration<double, std::milli>(endTime - startTime).count();
    
    // Gather statistics
    ParsingStats stats;
    stats.elapsedTimeMs = elapsedMs;
    stats.linesParsed = linesParsed;
    stats.threadsUsed = numThreads;
    stats.logLevelCounts = logLevelCounter.getCounts();
    
    return stats;
} 