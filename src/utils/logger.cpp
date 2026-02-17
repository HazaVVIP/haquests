#include "haquests/utils/logger.hpp"
#include <chrono>
#include <iomanip>
#include <ctime>

namespace haquests {
namespace utils {

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

Logger::Logger() : current_level_(LogLevel::INFO), console_output_(true) {}

Logger::~Logger() {
    if (file_output_.is_open()) {
        file_output_.close();
    }
}

void Logger::setLevel(LogLevel level) {
    current_level_ = level;
}

void Logger::setOutputFile(const std::string& filename) {
    if (file_output_.is_open()) {
        file_output_.close();
    }
    
    file_output_.open(filename, std::ios::app);
}

void Logger::setConsoleOutput(bool enabled) {
    console_output_ = enabled;
}

void Logger::debug(const std::string& message) {
    writeLog(LogLevel::DEBUG, message);
}

void Logger::info(const std::string& message) {
    writeLog(LogLevel::INFO, message);
}

void Logger::warning(const std::string& message) {
    writeLog(LogLevel::WARNING, message);
}

void Logger::error(const std::string& message) {
    writeLog(LogLevel::ERROR, message);
}

void Logger::fatal(const std::string& message) {
    writeLog(LogLevel::FATAL, message);
}

void Logger::writeLog(LogLevel level, const std::string& message) {
    if (level < current_level_) {
        return;
    }
    
    std::string log_line = "[" + getCurrentTime() + "] [" + 
                          levelToString(level) + "] " + message;
    
    if (console_output_) {
        if (level >= LogLevel::ERROR) {
            std::cerr << log_line << std::endl;
        } else {
            std::cout << log_line << std::endl;
        }
    }
    
    if (file_output_.is_open()) {
        file_output_ << log_line << std::endl;
        file_output_.flush();
    }
}

std::string Logger::levelToString(LogLevel level) const {
    switch (level) {
        case LogLevel::DEBUG:   return "DEBUG";
        case LogLevel::INFO:    return "INFO";
        case LogLevel::WARNING: return "WARNING";
        case LogLevel::ERROR:   return "ERROR";
        case LogLevel::FATAL:   return "FATAL";
        default:                return "UNKNOWN";
    }
}

std::string Logger::getCurrentTime() const {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    std::ostringstream oss;
    oss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    oss << '.' << std::setfill('0') << std::setw(3) << ms.count();
    
    return oss.str();
}

} // namespace utils
} // namespace haquests
