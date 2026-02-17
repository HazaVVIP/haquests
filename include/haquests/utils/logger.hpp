#pragma once

#include <string>
#include <sstream>
#include <iostream>
#include <fstream>

namespace haquests {
namespace utils {

enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    FATAL
};

class Logger {
public:
    static Logger& getInstance();
    
    // Set log level
    void setLevel(LogLevel level);
    
    // Set output file
    void setOutputFile(const std::string& filename);
    
    // Enable/disable console output
    void setConsoleOutput(bool enabled);
    
    // Log functions
    void debug(const std::string& message);
    void info(const std::string& message);
    void warning(const std::string& message);
    void error(const std::string& message);
    void fatal(const std::string& message);
    
    // Template log function
    template<typename... Args>
    void log(LogLevel level, Args&&... args) {
        if (level < current_level_) return;
        
        std::ostringstream oss;
        (oss << ... << args);
        writeLog(level, oss.str());
    }

private:
    Logger();
    ~Logger();
    
    void writeLog(LogLevel level, const std::string& message);
    std::string levelToString(LogLevel level) const;
    std::string getCurrentTime() const;
    
    LogLevel current_level_;
    bool console_output_;
    std::ofstream file_output_;
};

// Convenience macros
#define LOG_DEBUG(...) haquests::utils::Logger::getInstance().debug(__VA_ARGS__)
#define LOG_INFO(...) haquests::utils::Logger::getInstance().info(__VA_ARGS__)
#define LOG_WARNING(...) haquests::utils::Logger::getInstance().warning(__VA_ARGS__)
#define LOG_ERROR(...) haquests::utils::Logger::getInstance().error(__VA_ARGS__)
#define LOG_FATAL(...) haquests::utils::Logger::getInstance().fatal(__VA_ARGS__)

} // namespace utils
} // namespace haquests
