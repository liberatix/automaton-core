#ifndef AUTOMATON_CORE_SCRIPT_REGISTRY_H_
#define AUTOMATON_CORE_SCRIPT_REGISTRY_H_

#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include <boost/regex.hpp>

#include "automaton/core/common/obj.h"
#include "automaton/core/data/factory.h"
#include "automaton/core/data/msg.h"
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
  typedef common::obj * (*object_factory_function)(const data::msg& m);

  virtual const std::string name() const { return name_; }
  virtual const std::string name_with_api_version() const {
    return name_with_api_version(name_, api_version_);
  }
  virtual const std::string full_version() const { return version_; }
  virtual const uint32_t api_version() const { return api_version_; }
  virtual const uint32_t minor_version() const { return minor_version_; }
  virtual const uint32_t patch_version() const { return patch_version_; }
  virtual const std::string extra_version() const { return extra_version_; }

  virtual data::schema* schema() const = 0;

  virtual const std::vector<std::string> dependencies() const { return dependencies_; }
  virtual const std::vector<std::string> functions() const { return functions_; }
  virtual const std::vector<std::string> concepts() const { return concepts_; }
  virtual const std::unordered_map<std::string, object_factory_function> implementations() const {
    return implementations_;
  }

  // virtual void process(const data::msg& input, data::msg* output) = 0;

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

  void add_dependency(const std::string dependency, uint32_t api_version) {
    dependencies_.push_back(name_with_api_version(dependency, api_version));
  }

  void add_function(const std::string function) {
    functions_.push_back(function);
  }

  void add_concept(const std::string concept) {
    concepts_.push_back(concept);
  }

  void add_implementation(const std::string implementation, object_factory_function f) {
    if (implementations_.count(implementation) > 0) {
      std::stringstream ss;
      ss << "Module " << name() << " already has implementation for " << implementation;
      LOG(ERROR) << ss.str();
      throw ss.str();
    }
    implementations_[implementation] = f;
    // implementations_.push_back(implementation);
  }

 private:
  static const std::string name_with_api_version(const std::string name, uint32_t api_version) {
    return name + ".v" + std::to_string(api_version);
  }
  std::string name_;
  std::string version_;
  uint32_t api_version_;
  uint32_t minor_version_;
  uint32_t patch_version_;
  std::string extra_version_;
  std::vector<std::string> dependencies_;
  std::vector<std::string> functions_;
  std::vector<std::string> concepts_;
  // std::vector<std::string> implementations_;
  std::unordered_map<std::string, object_factory_function> implementations_;
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
    LOG(INFO) << "Importing module " << m.name() << " " << &(M::instance());
    if (modules_.count(m.name_with_api_version()) > 0) {
      LOG(ERROR) << "Module [" << m.name_with_api_version() << "] has already been imported!";
      throw "Already registered!";
    }
    modules_[m.name_with_api_version()] = &m;
    std::string json;
    m.schema()->to_json(&json);
    VLOG(9) << json;

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
    m.schema()->to_json(&json);

    // Import schema.
    LOG(INFO) << "Importing schema " << m.name_with_api_version();
    factory_->import_schema(m.schema(), m.name_with_api_version(), m.name_with_api_version());
  }

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

  data::factory& get_factory() { return *factory_.get() ; }
  common::obj* create(const data::msg& m);

  void process(const data::msg& request, data::msg* response);

 private:
  registry();

  std::unordered_map<std::string, module*> modules_;
  std::unique_ptr<data::factory> factory_;
  // std::unordered_map<uint64_t, common::obj> objects_;
};

}  // namespace script
}  // namespace core
}  // namespace automaton

#endif  // AUTOMATON_CORE_SCRIPT_REGISTRY_H_
