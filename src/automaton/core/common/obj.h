#ifndef AUTOMATON_CORE_COMMON_OBJ_H_
#define AUTOMATON_CORE_COMMON_OBJ_H_

#include <string>

namespace automaton {
namespace core {
namespace common {

struct status {
  enum code {
    OK = 0,
    CANCELLED = 1,
    UNKNOWN = 2,
    INVALID_ARGUMENT = 3,
    DEADLINE_EXCEEDED = 4,
    NOT_FOUND = 5,
    ALREADY_EXISTS = 6,
    PERMISSION_DENIED = 7,
    UNAUTHENTICATED = 16,
    RESOURCE_EXHAUSTED = 8,
    FAILED_PRECONDITION = 9,
    ABORTED = 10,
    OUT_OF_RANGE = 11,
    UNIMPLEMENTED = 12,
    INTERNAL = 13,
    UNAVAILABLE = 14,
    DATA_LOSS = 15,
  };

  explicit status(code error_code) : code_(error_code), msg_("") {}
  status(code error_code, std::string msg) : code_(error_code), msg_(msg) {}
  code code_;
  std::string msg_;
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

  virtual status process(const obj& request, obj* response) = 0;

 private:
  id id_;
};

}  // namespace common
}  // namespace core
}  // namespace automaton

#endif  // AUTOMATON_CORE_COMMON_OBJ_H_
