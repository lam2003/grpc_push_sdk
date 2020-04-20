#pragma once

#include <atomic>
#include <memory>

namespace EgcCommon {
class noncopyable {
 protected:
  noncopyable(void) {}
  ~noncopyable(void) {}

 private:  // emphasize the following members are private
  noncopyable(const noncopyable&) = delete;
  const noncopyable& operator=(const noncopyable&) = delete;
};

template <typename T>
class SingleTon : noncopyable {
 public:
  template <typename... ARGS>
  static std::shared_ptr<T> instance(ARGS... args) {
    if (!s_instantiated.test_and_set()) {
      s_instance_ = std::shared_ptr<T>(new T(std::forward<ARGS>(args)...));
      s_instantiating = false;
    }
    while (s_instantiating)
      ;
    return s_instance_;
  }
  static std::shared_ptr<T> instance(void) {
    if (!s_instantiated.test_and_set()) {
      s_instance_ = std::shared_ptr<T>(new T());
      s_instantiating = false;
    }
    while (s_instantiating)
      ;
    return s_instance_;
  }
  virtual ~SingleTon();

 protected:
  SingleTon(void){};

 private:
  static std::shared_ptr<T> s_instance_;
  static std::atomic_flag s_instantiated;
  static volatile bool s_instantiating;
};

template <typename T>
SingleTon<T>::~SingleTon() {}

template <typename T>
std::shared_ptr<T> SingleTon<T>::s_instance_ = nullptr;

template <typename T>
volatile bool SingleTon<T>::s_instantiating = true;

template <typename T>
std::atomic_flag SingleTon<T>::s_instantiated = ATOMIC_FLAG_INIT;

}  // namespace EgcCommon
