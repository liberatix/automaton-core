#include "automaton/core/cli/cli.h"

#include <cctype>
#include <iomanip>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <thread>
#include <utility>
#include <vector>

using Replxx = replxx::Replxx;

namespace automaton {
namespace core {
namespace cli {

Replxx::completions_t hook_completion(std::string const& context, int index, void* data) {
  Replxx::completions_t completions;
  return completions;
}

// NOLINTNEXTLINE
Replxx::hints_t hook_hint(std::string const& context, int index, Replxx::Color& c, void* data) {
  Replxx::hints_t hints;
  if (context.size() > 0) {
    std::stringstream str;
    str << "\n";
    for (auto i = 0; i < 8; i++) {
      str << "\n    Hint line " << i;
    }
    hints.push_back(str.str());
  }
  return hints;
}

// NOLINTNEXTLINE
void hook_color(std::string const& str, Replxx::colors_t& colors, void* data) {
}

cli::cli() {
  rx.install_window_change_handler();

  // set the max history size
  rx.set_max_history_size(1000);

  // set the max input line size
  rx.set_max_line_size(1024);

  // set the max number of hint rows to show
  rx.set_max_hint_rows(10);

  // set the callbacks
  rx.set_completion_callback(hook_completion, static_cast<void*>(this));
  rx.set_highlighter_callback(hook_color, static_cast<void*>(this));
  rx.set_hint_callback(hook_hint, static_cast<void*>(this));
}

char const* cli::input(const char * prompt) {
  char const* cinput{ nullptr };

  do {
    cinput = rx.input(prompt);
  } while ((cinput == nullptr) && (errno == EAGAIN));

  return cinput;
}

}  // namespace cli
}  // namespace core
}  // namespace automaton
