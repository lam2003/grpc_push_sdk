#ifndef EDU_PUSH_SDK_LOG_H
#define EDU_PUSH_SDK_LOG_H

#include <memory>
#include <string>

//不使用threadlocal, 否则ios工具链无法编译
#ifndef SPDLOG_NO_TLS
#    define SPDLOG_NO_TLS
#endif
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#define SDK_LOG_PREFIX ": "
#define SDK_LOG_PREFIX_DEBUG "[{}:{}][{}]: "

namespace edu {

enum class LOG_LEVEL {
    TRACE    = 0,
    DEBUG    = 1,
    INFO     = 2,
    WARN     = 3,
    ERROR    = 4,
    CRITICAL = 5,
    OFF      = 6
};

class Log final {
  public:
    Log(const std::string& name);
    ~Log();

  public:
    // 需要在初始化之前调用
    void LogOnConsole(bool v);
    void SetOutputDir(const std::string& dir);
    void SetLogLevel(LOG_LEVEL level);

    int Initialize();

    template <typename... ARGS> void Trace(const std::string& fmt, ARGS... args)
    {
        if (console_logger_)
            console_logger_->trace(fmt, std::forward<ARGS>(args)...);
        if (file_logger_)
            file_logger_->trace(fmt, std::forward<ARGS>(args)...);
    }
    template <typename... ARGS> void Debug(const std::string& fmt, ARGS... args)
    {
        if (console_logger_)
            console_logger_->debug(fmt, std::forward<ARGS>(args)...);
        if (file_logger_)
            file_logger_->debug(fmt, std::forward<ARGS>(args)...);
    }
    template <typename... ARGS> void Info(const std::string& fmt, ARGS... args)
    {
        if (console_logger_)
            console_logger_->info(fmt, std::forward<ARGS>(args)...);
        if (file_logger_)
            file_logger_->info(fmt, std::forward<ARGS>(args)...);
    }
    template <typename... ARGS> void Warn(const std::string& fmt, ARGS... args)
    {
        if (console_logger_)
            console_logger_->warn(fmt, std::forward<ARGS>(args)...);
        if (file_logger_)
            file_logger_->warn(fmt, std::forward<ARGS>(args)...);
    }
    template <typename... ARGS> void Error(const std::string& fmt, ARGS... args)
    {
        if (console_logger_)
            console_logger_->error(fmt, std::forward<ARGS>(args)...);
        if (file_logger_)
            file_logger_->error(fmt, std::forward<ARGS>(args)...);
    }
    template <typename... ARGS>
    void Critical(const std::string& fmt, ARGS... args)
    {
        if (console_logger_)
            console_logger_->critical(fmt, std::forward<ARGS>(args)...);
        if (file_logger_)
            file_logger_->critical(fmt, std::forward<ARGS>(args)...);
    }

  private:
    std::shared_ptr<spdlog::logger> file_logger_;
    std::shared_ptr<spdlog::logger> console_logger_;
    bool                            log_on_console_;
    LOG_LEVEL                       log_level_;
    std::string                     format_;
    std::string                     dir_;
    std::string                     logger_name_;
};

}  // namespace edu

extern std::shared_ptr<edu::Log> _sdk_logger;
extern std::shared_ptr<edu::Log> _grpc_logger;
extern int                       init_logger(const std::string& log_dir);

#if PUSH_SDK_DEBUG
#    define log_e(msg, ...)                                                    \
        do {                                                                   \
            if (_sdk_logger) {                                                 \
                _sdk_logger->Error(SDK_LOG_PREFIX_DEBUG + std::string(msg),    \
                                   __FILE__, __LINE__, __FUNCTION__,           \
                                   ##__VA_ARGS__);                             \
            }                                                                  \
        } while (0)
#    define log_t(msg, ...)                                                    \
        do {                                                                   \
            if (_sdk_logger) {                                                 \
                _sdk_logger->Trace(SDK_LOG_PREFIX_DEBUG + std::string(msg),    \
                                   __FILE__, __LINE__, __FUNCTION__,           \
                                   ##__VA_ARGS__);                             \
            }                                                                  \
        } while (0)
#    define log_i(msg, ...)                                                    \
        do {                                                                   \
            if (_sdk_logger) {                                                 \
                _sdk_logger->Info(SDK_LOG_PREFIX_DEBUG + std::string(msg),     \
                                  __FILE__, __LINE__, __FUNCTION__,            \
                                  ##__VA_ARGS__);                              \
            }                                                                  \
        } while (0)
#    define log_w(msg, ...)                                                    \
        do {                                                                   \
            if (_sdk_logger) {                                                 \
                _sdk_logger->Warn(SDK_LOG_PREFIX_DEBUG + std::string(msg),     \
                                  __FILE__, __LINE__, __FUNCTION__,            \
                                  ##__VA_ARGS__);                              \
            }                                                                  \
        } while (0)
#    define log_c(msg, ...)                                                    \
        do {                                                                   \
            if (_sdk_logger) {                                                 \
                _sdk_logger->Critical(SDK_LOG_PREFIX_DEBUG + std::string(msg), \
                                      __FILE__, __LINE__, __FUNCTION__,        \
                                      ##__VA_ARGS__);                          \
            }                                                                  \
        } while (0)
#    define log_d(msg, ...)                                                    \
        do {                                                                   \
            if (_sdk_logger) {                                                 \
                _sdk_logger->Debug(SDK_LOG_PREFIX_DEBUG + std::string(msg),    \
                                   __FILE__, __LINE__, __FUNCTION__,           \
                                   ##__VA_ARGS__);                             \
            }                                                                  \
        } while (0)
#else
#    define log_e(msg, ...)                                                    \
        do {                                                                   \
            if (_sdk_logger) {                                                 \
                _sdk_logger->Error(SDK_LOG_PREFIX + std::string(msg),          \
                                   ##__VA_ARGS__);                             \
            }                                                                  \
        } while (0)
#    define log_t(msg, ...)                                                    \
        do {                                                                   \
            if (_sdk_logger) {                                                 \
                _sdk_logger->Trace(SDK_LOG_PREFIX + std::string(msg),          \
                                   ##__VA_ARGS__);                             \
            }                                                                  \
        } while (0)
#    define log_i(msg, ...)                                                    \
        do {                                                                   \
            if (_sdk_logger) {                                                 \
                _sdk_logger->Info(SDK_LOG_PREFIX + std::string(msg),           \
                                  ##__VA_ARGS__);                              \
            }                                                                  \
        } while (0)
#    define log_w(msg, ...)                                                    \
        do {                                                                   \
            if (_sdk_logger) {                                                 \
                _sdk_logger->Warn(SDK_LOG_PREFIX + std::string(msg),           \
                                  ##__VA_ARGS__);                              \
            }                                                                  \
        } while (0)
#    define log_c(msg, ...)                                                    \
        do {                                                                   \
            if (_sdk_logger) {                                                 \
                _sdk_logger->Critical(SDK_LOG_PREFIX + std::string(msg),       \
                                      ##__VA_ARGS__);                          \
            }                                                                  \
        } while (0)
#    define log_d(msg, ...)                                                    \
        do {                                                                   \
            if (_sdk_logger) {                                                 \
                _sdk_logger->Debug(SDK_LOG_PREFIX + std::string(msg),          \
                                   ##__VA_ARGS__);                             \
            }                                                                  \
        } while (0)
#endif

#endif