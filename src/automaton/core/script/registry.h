#ifndef AUTOMATON_CORE_SCRIPT_REGISTRY_H_
#define AUTOMATON_CORE_SCRIPT_REGISTRY_H_

#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include <boost/regex.hpp>

#include "automaton/core/data/factory.h"
#include "automaton/core/data/schema.h"
#include "automaton/core/log/log.h"

namespace automaton {
namespace core {
namespace script {

/**
  Module interface definition base class.
*/
class module {
 public:
  virtual const std::string name() const { return name_; }
  virtual const std::string name_with_api_version() const {
    return name_with_api_version(name_, api_version_);
  }
  virtual const std::string full_version() const { return version_; }
  virtual const uint32_t api_version() const { return api_version_; }
  virtual const uint32_t minor_version() const { return minor_version_; }
  virtual const uint32_t patch_version() const { return patch_version_; }
  virtual const std::string extra_version() const { return extra_version_; }
  virtual const std::vector<std::string> dependencies() const { return dependencies_; }
  virtual data::schema* schema() const = 0;

  virtual void add_dependency(const std::string dependency, uint32_t api_version) {
    dependencies_.push_back(name_with_api_version(dependency, api_version));
  }

 protected:
  module(const std::string name, const std::string version) : name_(name), version_(version) {
    static const boost::regex version_regex{"^(\\d+)(\\.\\d+)(\\.\\d+)(\\..*)$"};
    boost::smatch what;
    if (boost::regex_search(version_, what, version_regex)) {
      api_version_ = std::atoi(what[1].str().c_str());
      minor_version_ = std::atoi(what[2].str().c_str());
      patch_version_ = std::atoi(what[3].str().c_str());
      extra_version_ = what[4].str();
    } else {
      // TODO(asen): Trhow an actual exception.
      throw "Invalid version!";
    }
  }

 private:
  static const std::string name_with_api_version(const std::string name,
                                                   uint32_t api_version) {
    return name + "_v" + std::to_string(api_version);
  }
  std::string name_;
  std::string version_;
  uint32_t api_version_;
  uint32_t minor_version_;
  uint32_t patch_version_;
  std::string extra_version_;
  std::vector<std::string> dependencies_;
};

/**
  Registry for scriptable modules.
*/
class registry {
 public:
  registry(registry&) = delete;
  registry(const registry&) = delete;

  /**
    Binds module M to the script::registry.
  */
  template<typename M>
  void import() {
    M& m = M::instance();
    LOG(INFO) << "Importing module " << m.name();
    if (modules_.count(m.name_with_api_version()) > 0) {
      LOG(ERROR) << "Module [" << m.name_with_api_version() << "] has already been imported!";
      throw "Already registered!";
    }
    modules_[m.name_with_api_version()] = &m;

    // Ensure dependencies are already imported.
    for (const auto& dep : m.dependencies()) {
      if (modules_.count(dep) == 0) {
        LOG(ERROR) << m.name_with_api_version() << " depends on "
            << dep << " which hasn't been imported yet!";
        throw "dependency issue!";
      }
      const auto& dep_module = modules_[dep];
      m.schema()->add_dependency(dep_module->name_with_api_version());
    }

    // Import schema.
    factory_->import_schema(m.schema(), m.name_with_api_version(), m.name_with_api_version());  // m.name());
  }

  void configure();

  /**
    Dumps information about all registered modules, functions, classes and schemas into a string.
  */
  std::string to_string();

  /**
    Gets reference to the registry instance.

    Creates the instance when called for the first time.
  */
  static registry& instance() {
    LOG(INFO) << "Creating registry.";
    static registry inst;
    return inst;
  }

 private:
  registry();

  std::unordered_map<std::string, module*> modules_;
  std::unique_ptr<data::factory> factory_;
};

}  // namespace script
}  // namespace core
}  // namespace automaton

#endif  // AUTOMATON_CORE_SCRIPT_REGISTRY_H_
