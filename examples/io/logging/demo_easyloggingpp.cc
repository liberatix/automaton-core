#include "src/easylogging++.h"

INITIALIZE_EASYLOGGINGPP

int main() {
  std::cout << "\nYou should see only the appropriate logs!\n\n";
  el::Loggers::addFlag(el::LoggingFlag::ColoredTerminalOutput);

  el::Loggers::addFlag(el::LoggingFlag::HierarchicalLogging);
  // el::Loggers::setLoggingLevel(el::Level::Trace);
  // el::Loggers::setLoggingLevel(el::Level::Info);
  el::Loggers::setLoggingLevel(el::Level::Error);

  LOG(TRACE) << "This is trace log!";
  LOG(DEBUG) << "This is debug log!";
  LOG(INFO) << "This is info log!";
  LOG(WARNING) << "This is warning log!";
  LOG(ERROR) << "This is error log!";

  return 0;
}
