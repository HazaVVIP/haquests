#include "haquests/utils/timer.hpp"
#include <iostream>

namespace haquests {
namespace utils {

Timer::Timer() : running_(false), timeout_(0) {}

void Timer::start() {
    start_time_ = Clock::now();
    running_ = true;
}

void Timer::stop() {
    running_ = false;
}

void Timer::reset() {
    start_time_ = Clock::now();
    running_ = true;
}

Timer::Duration Timer::elapsed() const {
    if (!running_) {
        return Duration(0);
    }
    
    auto now = Clock::now();
    return std::chrono::duration_cast<Duration>(now - start_time_);
}

bool Timer::hasExpired(Duration timeout) const {
    return elapsed() >= timeout;
}

void Timer::setTimeout(Duration timeout, Callback callback) {
    timeout_ = timeout;
    callback_ = callback;
}

void Timer::checkTimeout() {
    if (running_ && callback_ && hasExpired(timeout_)) {
        callback_();
        stop();
    }
}

ScopeTimer::ScopeTimer(const char* name) : name_(name) {
    start_ = Timer::Clock::now();
}

ScopeTimer::~ScopeTimer() {
    auto end = Timer::Clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start_);
    std::cout << "[Timer] " << name_ << ": " << duration.count() << "ms" << std::endl;
}

} // namespace utils
} // namespace haquests
