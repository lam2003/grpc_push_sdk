#ifndef EDU_SERVICE_MESH_SINGLETON_H
#define EDU_SERVICE_MESH_SINGLETON_H

#include <atomic>
#include <memory>

//线程安全的单例
namespace edu {
class noncopyable {
  protected:
    noncopyable(void) {}
    ~noncopyable(void) {}

  private:
    noncopyable(const noncopyable&) = delete;
    noncopyable& operator=(const noncopyable&) = delete;
};

template <typename T> class Singleton : public noncopyable {
  public:
    template <typename... ARGS> static std::shared_ptr<T> Instance(ARGS... args)
    {
        if (!instantiated_.test_and_set()) {
            instance_ = std::shared_ptr<T>(new T(std::forward<ARGS>(args)...));
            initializing_ = false;
        }
        while (initializing_ == true) {}
        return instance_;
    }

    static std::shared_ptr<T> Instance()
    {
        if (!instantiated_.test_and_set()) {
            instance_     = std::shared_ptr<T>(new T);
            initializing_ = false;
        }
        while (initializing_ == true) {}
        return instance_;
    }

    virtual ~Singleton() {}

  protected:
    Singleton() {}

  private:
    static std::atomic_flag   instantiated_;
    static volatile bool      initializing_;
    static std::shared_ptr<T> instance_;
};

template <typename T>
std::atomic_flag Singleton<T>::instantiated_ = ATOMIC_FLAG_INIT;
template <typename T> volatile bool      Singleton<T>::initializing_ = true;
template <typename T> std::shared_ptr<T> Singleton<T>::instance_     = nullptr;

}  // namespace edu
#endif