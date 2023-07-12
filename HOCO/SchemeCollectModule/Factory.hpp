#ifndef __FACTORY_H__
#define __FACTORY_H__

#pragma once
#include <functional>
#include "../Utils/utils.hpp"

template<class B>
class Factory {
    /* A registration map */
  std::map<std::string, std::function<B*()>> s_creators;

public:

  static Factory<B>& getInstance() {
    static Factory<B> s_instance;
    return s_instance;
  }

  template<class T>
  void registerClass(const std::string& name) {
    s_creators.insert({name, []() -> B* { return new T(); }});
  }

  void unregisterClass(const std::string& name) {
    s_creators.erase(name);
  }

  B* create(const std::string& name) {
    const auto it = s_creators.find(name);
    if (it == s_creators.end()) {
        cout << "null" << endl;
        return nullptr; // not a derived class
    }
    return (it->second)();
  }

  void printRegisteredClasses() {
    for (const auto &creator : s_creators) {
      std::cout << creator.first << endl;
    }
  }
};

#define FACTORY(Class, ...) Factory<Class<__VA_ARGS__>>::getInstance()

template <template <typename...> class B, template <typename...> class T, typename... Args>
class Creator {
public:
  explicit Creator(const std::string& name) {
    // FACTORY(B).registerClass<T>(name);
    FACTORY(B, Args...).template registerClass<T<Args...>> (name);    // to correctly invoke function with template   
                                                    // use the template keyword before the function name
                                                    // to indicate that it is a dependent template function call. 
  }
};

template <template <typename...> class B, template <typename...> class T, typename... Args>
class Destroyer {
public:
  explicit Destroyer(const std::string& name) {
    FACTORY(B, Args...).template unregisterClass(name);
  }
};

#define REGISTER(base_class, derived_class, ...) \
  Creator<base_class, derived_class, ##__VA_ARGS__> s_##derived_class##Creator(#derived_class);

#define UNREGISTER(base_class, derived_class, ...) \
  Destroyer<base_class, derived_class, ##__VA_ARGS__> s_##derived_class##Destroyer(#derived_class);

#endif