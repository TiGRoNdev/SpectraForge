#include "SpectraForge/Headless/Logger.h"

#include <chrono>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace SpectraForge::Headless {

namespace {
std::string timestamp() {
    const auto now = std::chrono::system_clock::now();
    const auto time = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
#ifdef _WIN32
    localtime_s(&tm, &time);
#else
    localtime_r(&time, &tm);
#endif
    std::ostringstream oss;
    oss << std::put_time(&tm, "%H:%M:%S");
    return oss.str();
}
}  // namespace

Logger::Logger(std::string name) : name_(std::move(name)) {}

void Logger::info(const std::string& message) { log("INFO", message); }

void Logger::warn(const std::string& message) { log("WARN", message); }

void Logger::error(const std::string& message) { log("ERROR", message); }

std::vector<std::string> Logger::history() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return entries_;
}

void Logger::log(const std::string& level, const std::string& message) {
    const std::string entry = "[" + timestamp() + "] [" + name_ + "] [" + level + "] " + message;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        entries_.push_back(entry);
    }
    std::cout << entry << std::endl;
}

}  // namespace SpectraForge::Headless
