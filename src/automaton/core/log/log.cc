#include "automaton/core/log/log.h"

#include <string>

INITIALIZE_EASYLOGGINGPP

namespace core {
namespace log {

bool init_logger() {
  el::Loggers::reconfigureAllLoggers(el::ConfigurationType::Format,
      std::string("%datetime %levshort [%logger] [%fbase:%line]: %msg"));

  el::Loggers::addFlag(el::LoggingFlag::ColoredTerminalOutput);

  el::Loggers::setLoggingLevel(el::Level::Global);
  // el::Loggers::setVerboseLevel(9);

  return true;
}

bool _init_logger = init_logger();

}  // namespace log
}  // namespace core
