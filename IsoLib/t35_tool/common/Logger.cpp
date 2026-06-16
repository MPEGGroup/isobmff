#include "Logger.hpp"

namespace t35
{

std::shared_ptr<spdlog::logger> Logger::logger = nullptr;

void Logger::init(int verboseLevel)
{
  if(logger)
  {
    return; // Already initialized
  }

  // Create console logger with color
  logger = spdlog::stdout_color_mt("t35_tool");

  // Set level based on verbosity
  switch(verboseLevel)
  {
  case 0:
    logger->set_level(spdlog::level::err);
    break;
  case 1:
    logger->set_level(spdlog::level::warn);
    break;
  case 2:
    logger->set_level(spdlog::level::info);
    break;
  case 3:
  default:
    logger->set_level(spdlog::level::debug);
    break;
  }

  // Set pattern: [LEVEL] message
  logger->set_pattern("[%^%l%$] %v");

  LOG_DEBUG("Logger initialized with verbose level {}", verboseLevel);
}

std::shared_ptr<spdlog::logger> Logger::get()
{
  if(!logger)
  {
    init(); // Auto-initialize with default level if not already done
  }
  return logger;
}

} // namespace t35
