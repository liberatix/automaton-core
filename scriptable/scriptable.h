#ifndef AUTOMATON_CORE_SCRIPTABLE_SCRIPTABLE_H__
#define AUTOMATON_CORE_SCRIPTABLE_SCRIPTABLE_H__

#include <iostream>
#include <map>
#include <string>

#include "schema/schema.h"

/** Scriptable interface template

    The scriptable interface T must have the following two methods in order to
    

    ```
    scriptable<interface>::register(schema);
    ```
*/
template<class T>
class scriptable {
 public:
  /** Registers an interface definition and makes it scriptable.
  */
  static register_factory(const std::string interface_name,
                          const schema::schema_definition& schema);

 private:
  static std::map<std::string, scriptable_factory_function_type>
    scriptable_factory;
};

#endif  // AUTOMATON_CORE_SCRIPTABLE_SCRIPTABLE_H__
