#pragma once

#include <chrono>
#include <functional>

namespace haquests {
namespace utils {

class Timer {
public:
    using Clock = std::chrono::steady_clock;
    using TimePoint = Clock::time_point;
    using Duration = std::chrono::milliseconds;
    using Callback = std::function<void()>;
    
    Timer();
    
    // Start timer
    void start();
    
    // Stop timer
    void stop();
    
    // Reset timer
    void reset();
    
    // Get elapsed time
    Duration elapsed() const;
    
    // Check if timeout occurred
    bool hasExpired(Duration timeout) const;
    
    // Set timeout callback
    void setTimeout(Duration timeout, Callback callback);
    
    // Check and execute timeout
    void checkTimeout();

private:
    TimePoint start_time_;
    bool running_;
    Duration timeout_;
    Callback callback_;
};

// Scope timer for profiling
class ScopeTimer {
public:
    explicit ScopeTimer(const char* name);
    ~ScopeTimer();
    
private:
    const char* name_;
    Timer::TimePoint start_;
};

} // namespace utils
} // namespace haquests
