#ifndef AUTOMATON_CORE_COMMON_OBJ_H_
#define AUTOMATON_CORE_COMMON_OBJ_H_

#include <string>

namespace automaton {
namespace core {
namespace common {

enum status_code {
  OK = 0,
};

struct status {
  // NOLINTNEXTLINE
  status(status_code code) : code(code), msg("") {}
  status(status_code code, std::string msg) : code(code), msg(msg) {}
  status_code code;
  std::string msg;
};

/**
  Object instance wrapper.
*/
class obj {
 public:
  virtual ~obj() {}

  typedef uint64_t id;

  id get_id() {
    return id_;
  }

  virtual status process(const obj& request, obj** response) = 0;

 private:
  id id_;
};

}  // namespace common
}  // namespace core
}  // namespace automaton

#endif  // AUTOMATON_CORE_COMMON_OBJ_H_
