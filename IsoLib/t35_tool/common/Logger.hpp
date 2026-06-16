#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <memory>

namespace t35 {

/**
 * Logger wrapper for t35_tool using spdlog
 * Provides simple logging interface with various levels
 */
class Logger {
public:
    /**
     * Initialize logger (call once at start of program)
     * @param verboseLevel 0=error only, 1=warn+error, 2=info+warn+error, 3=debug+all
     */
    static void init(int verboseLevel = 2);

    /**
     * Get the logger instance
     */
    static std::shared_ptr<spdlog::logger> get();

    // Convenience logging methods
    template<typename... Args>
    static void debug(Args&&... args) {
        get()->debug(std::forward<Args>(args)...);
    }

    template<typename... Args>
    static void info(Args&&... args) {
        get()->info(std::forward<Args>(args)...);
    }

    template<typename... Args>
    static void warn(Args&&... args) {
        get()->warn(std::forward<Args>(args)...);
    }

    template<typename... Args>
    static void error(Args&&... args) {
        get()->error(std::forward<Args>(args)...);
    }

    template<typename... Args>
    static void critical(Args&&... args) {
        get()->critical(std::forward<Args>(args)...);
    }

private:
    static std::shared_ptr<spdlog::logger> logger;
};

} // namespace t35

// Convenience macros
#define LOG_DEBUG(...) t35::Logger::debug(__VA_ARGS__)
#define LOG_INFO(...)  t35::Logger::info(__VA_ARGS__)
#define LOG_WARN(...)  t35::Logger::warn(__VA_ARGS__)
#define LOG_ERROR(...) t35::Logger::error(__VA_ARGS__)
#define LOG_CRITICAL(...) t35::Logger::critical(__VA_ARGS__)
