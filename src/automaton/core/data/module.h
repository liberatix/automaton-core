#ifndef AUTOMATON_CORE_DATA_MODULE_H__
#define AUTOMATON_CORE_DATA_MODULE_H__

namespace automaton {
namespace core {
namespace data {

struct module {
  static constexpr auto name = "data";
  static constexpr auto version = "0.0.1";
  static bool registered;
};

}  // namespace data
}  // namespace core
}  // namespace automaton

#endif  // AUTOMATON_CORE_DATA_MODULE_H__
