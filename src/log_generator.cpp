#include "log_generator.h"
#include <iostream>
#include <fstream>
#include <ctime>
#include <thread>
#include <vector>
#include <mutex>
#include <atomic>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <random>
#include <condition_variable>
#include <queue>

// Thread-safe queue for task distribution
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

    bool isEmpty() {
        std::unique_lock<std::mutex> lock(mutex);
        return queue.empty();
    }
};

// Thread pool for efficient task distribution
class ThreadPool {
private:
    std::vector<std::thread> workers;
    ThreadSafeQueue<std::function<void()>> tasks;
    std::atomic<bool> stop{false};

public:
    ThreadPool(size_t numThreads) {
        for (size_t i = 0; i < numThreads; ++i) {
            workers.emplace_back([this] {
                std::function<void()> task;
                while (tasks.pop(task)) {
                    if (stop) break;
                    task();
                }
            });
        }
    }

    ~ThreadPool() {
        stop = true;
        tasks.finish();
        for (auto& worker : workers) {
            if (worker.joinable()) {
                worker.join();
            }
        }
    }

    template<typename F>
    void enqueue(F&& f) {
        tasks.push(std::forward<F>(f));
    }

    size_t size() const {
        return workers.size();
    }
};

// File writer with buffering for improved performance
class BufferedFileWriter {
private:
    std::ofstream file;
    std::mutex fileMutex;
    std::vector<std::string> buffer;
    const size_t bufferSize;
    std::mutex bufferMutex;

public:
    BufferedFileWriter(const std::string& filename, size_t bufferSize = 1000) 
        : bufferSize(bufferSize) {
        file.open(filename, std::ios::trunc);
        if (!file) {
            throw std::runtime_error("Could not open file: " + filename);
        }
        buffer.reserve(bufferSize);
    }

    ~BufferedFileWriter() {
        flush();
        file.close();
    }

    void write(const std::string& line) {
        std::lock_guard<std::mutex> lock(bufferMutex);
        buffer.push_back(line);
        
        if (buffer.size() >= bufferSize) {
            flush();
        }
    }

    void flush() {
        std::vector<std::string> tempBuffer;
        {
            std::lock_guard<std::mutex> lock(bufferMutex);
            if (buffer.empty()) return;
            tempBuffer.swap(buffer);
        }

        std::lock_guard<std::mutex> lock(fileMutex);
        for (const auto& line : tempBuffer) {
            file << line;
        }
        file.flush();
    }
};

// Get timestamp with high precision
std::string getTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto now_time_t = std::chrono::system_clock::to_time_t(now);
    auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&now_time_t), "%Y-%m-%d %H:%M:%S");
    ss << '.' << std::setfill('0') << std::setw(3) << now_ms.count();
    return ss.str();
}

// Get optimal thread count based on hardware
int getOptimalThreadCount() {
    int threadCount = std::thread::hardware_concurrency();
    return threadCount > 0 ? threadCount : 4; // Default to 4 if detection fails
}

// Generate logs with optimized multithreading
GenerationStats generateLogs(const std::string &filename, int count, int numThreads) {
    auto startTime = std::chrono::high_resolution_clock::now();

    // Use optimal thread count if not specified
    if (numThreads <= 0) {
        numThreads = getOptimalThreadCount();
    }

    // Cap threads to a reasonable maximum
    numThreads = std::min(numThreads, 32);
    
    ThreadPool pool(numThreads);
    BufferedFileWriter writer(filename);
    std::atomic<int> completedLogs(0);
    
    // Log level distribution
    const std::vector<std::string> logLevels = {"INFO", "WARNING", "ERROR", "DEBUG"};
    
    // Set up random number generator
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> levelDist(0, logLevels.size() - 1);
    
    // Calculate chunk size for optimal distribution
    const int chunkSize = std::max(100, count / (numThreads * 10));
    
    for (int startIdx = 0; startIdx < count; startIdx += chunkSize) {
        int endIdx = std::min(startIdx + chunkSize, count);
        
        pool.enqueue([startIdx, endIdx, &completedLogs, &logLevels, &writer, &levelDist, &gen]() {
            thread_local std::mt19937 localGen(std::random_device{}() + startIdx);
            
            for (int i = startIdx; i < endIdx; ++i) {
                std::string timestamp = getTimestamp();
                std::string level = logLevels[levelDist(localGen)];
                
                std::stringstream line;
                line << timestamp << " [" << level << "] Log entry #" << (i + 1) << "\n";
                
                writer.write(line.str());
                completedLogs++;
            }
        });
    }
    
    // Wait until all logs are completed (pool destructor will join all threads)
    while (completedLogs < count) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    // Ensure any remaining logs are flushed
    writer.flush();
    
    // Calculate elapsed time
    auto endTime = std::chrono::high_resolution_clock::now();
    double elapsedMs = std::chrono::duration<double, std::milli>(endTime - startTime).count();
    
    // Return statistics
    GenerationStats stats;
    stats.elapsedTimeMs = elapsedMs;
    stats.logsGenerated = count;
    stats.threadsUsed = numThreads;
    
    return stats;
}
