#include "automaton/core/log/log.h"

#include <string>

INITIALIZE_EASYLOGGINGPP

namespace automaton {
namespace core {

namespace io {

// TODO(asen): consider merging log and io modules
// From io module, preventing circular reference.
bool file_exists(const char* filename);

}  // namespace io

namespace log {

const char* DEFAULT_LOG_CONFIG_FILENAME = "log.cfg";
const char* DEFAILT_LOG_OUTPUT_FILENAME = "automaton-core.log";

bool init_logger() {
  // Load configuration from file
  if (automaton::core::io::file_exists(DEFAULT_LOG_CONFIG_FILENAME)) {
    el::Configurations conf(DEFAULT_LOG_CONFIG_FILENAME);
  } else {
    el::Loggers::reconfigureAllLoggers(el::ConfigurationType::Format,
        std::string("%datetime %levshort [%fbase:%line]: %msg"));

    el::Loggers::reconfigureAllLoggers(el::ConfigurationType::ToFile, "true");
    el::Loggers::reconfigureAllLoggers(el::ConfigurationType::ToStandardOutput, "false");
    el::Loggers::reconfigureAllLoggers(
        el::ConfigurationType::Filename, DEFAILT_LOG_OUTPUT_FILENAME);

    el::Loggers::setLoggingLevel(el::Level::Global);
    // el::Loggers::setVerboseLevel(9);
  }

  el::Loggers::addFlag(el::LoggingFlag::ColoredTerminalOutput);
  el::Loggers::addFlag(el::LoggingFlag::LogDetailedCrashReason);

  return true;
}

bool _init_logger = init_logger();

}  // namespace log
}  // namespace core
}  // namespace automaton
