#pragma once

#include <mutex>
#include <string>
#include <vector>

namespace SpectraForge::Headless {

class Logger {
  public:
    explicit Logger(std::string name = "Headless");

    void info(const std::string& message);
    void warn(const std::string& message);
    void error(const std::string& message);

    [[nodiscard]] std::vector<std::string> history() const;

  private:
    void log(const std::string& level, const std::string& message);

    std::string name_;
    mutable std::mutex mutex_;
    std::vector<std::string> entries_;
};

}  // namespace SpectraForge::Headless
