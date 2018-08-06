#include <iostream>
#include <string>

#include "automaton/core/cli/cli.h"

void string_replace(std::string* str,
                    const std::string& oldStr,
                    const std::string& newStr) {
  std::string::size_type pos = 0u;
  while ((pos = str->find(oldStr, pos)) != std::string::npos) {
     str->replace(pos, oldStr.length(), newStr);
     pos += newStr.length();
  }
}

static std::string automaton_ascii_logo =
  "\n\x1b[40m\x1b[1m"
  "                                                                   " "\x1b[0m\n\x1b[40m\x1b[1m"
  "                                                                   " "\x1b[0m\n\x1b[40m\x1b[1m"
  "   @197m█▀▀▀█ @39m█ █ █ @11m▀▀█▀▀ @129m█▀▀▀█ @47m█▀█▀█ @9m█▀▀▀█ @27m▀▀█▀▀ @154m█▀▀▀█ @13m█▀█ █           " "\x1b[0m\n\x1b[40m\x1b[1m" // NOLINT
  "   @197m█▀▀▀█ @39m█ ▀ █ @11m█ █ █ @129m█ ▀ █ @47m█ ▀ █ @9m█▀▀▀█ @27m█ █ █ @154m█ ▀ █ @13m█ █ █  @15mCORE     " "\x1b[0m\n\x1b[40m\x1b[1m" // NOLINT
  "   @197m▀ ▀ ▀ @39m▀▀▀▀▀ @11m▀ ▀ ▀ @129m▀▀▀▀▀ @47m▀ ▀ ▀ @9m▀ ▀ ▀ @27m▀ ▀ ▀ @154m▀▀▀▀▀ @13m▀ ▀▀▀  @15mv0.0.1   " "\x1b[0m\n\x1b[40m\x1b[1m" // NOLINT
  "                                                                   " "\x1b[0m\n\x1b[40m\x1b[1m"
  "                                                                   " "\x1b[0m\n\x1b[40m\x1b[1m"
  "   @197m█▀▀▀█ @39m█   █ @11m▀▀█▀▀ @129m█▀▀▀█ @47m█▀█▀█ @9m█▀▀▀█ @27m▀▀█▀▀ @154m█▀▀▀█ @13m█▀█ █           " "\x1b[0m\n\x1b[40m\x1b[1m" // NOLINT
  "   @197m█▀▀▀█ @39m█   █ @11m  █   @129m█   █ @47m█ ▀ █ @9m█▀▀▀█ @27m  █   @154m█   █ @13m█ █ █  @15mCORE     " "\x1b[0m\n\x1b[40m\x1b[1m" // NOLINT
  "   @197m▀   ▀ @39m▀▀▀▀▀ @11m  ▀   @129m▀▀▀▀▀ @47m▀   ▀ @9m▀   ▀ @27m  ▀   @154m▀▀▀▀▀ @13m▀ ▀▀▀  @15mv0.0.1   " "\x1b[0m\n\x1b[40m\x1b[1m" // NOLINT
  "                                                                   " "\x1b[0m\n\n"
  "  @7mThese are common Automaton commands used in various situations:\n"
  "\n"
  "     \x1b[1m@15m.modules    \x1b[0m@7mShow list of registered modules\n"
  "     \x1b[1m@15m.protos     \x1b[0m@7mShow list of registered smart protocol definitions\n"
  "     \x1b[1m@15m.nodes      \x1b[0m@7mShow list of node instances running on this client\n"
  "     \x1b[1m@15m.launch     \x1b[0m@7mLaunch a smart protocol node instance from a definiition\n"
  "     \x1b[1m@15m.use        \x1b[0m@7mSet the current smart protocol node\n"
  "     \x1b[1m@15m.msg        \x1b[0m@7mConstruct and send a message to the current smart protocol\n\n";

int main(int argc, char* argv[]) {
  string_replace(&automaton_ascii_logo, "@", "\x1b[38;5;");

  automaton::core::cli::cli cli;
  std::cout << automaton_ascii_logo;
  while (1) {
    auto cmd = cli.input("\x1b[38;5;15m\x1b[1m|A|\x1b[0m ");
    if (cmd == nullptr) {
      printf("\n");
      break;
    }
  }
  return 0;
}
