#include <common/log.h>
#include <common/utils.h>

#include <sstream>

#include <ServiceTransPushIf.h>

#define DEFAULT_FORMAT "[%Y-%m-%d %H:%M:%S.%e][%t][%l]%v%$"

namespace edu {

Log::Log(const std::string& name)
{
    file_logger_    = nullptr;
    console_logger_ = nullptr;
    log_on_console_ = false;
    log_level_      = LOG_LEVEL::TRACE;
    format_         = DEFAULT_FORMAT;
    dir_            = "";
    logger_name_    = name;
}

Log::~Log()
{
    if (console_logger_) {
        console_logger_.reset();
        console_logger_ = nullptr;
    }

    if (file_logger_) {
        file_logger_.reset();
        file_logger_ = nullptr;
    }
}

void Log::LogOnConsole(bool v)
{
    log_on_console_ = v;
}

void Log::SetOutputDir(const std::string& dir)
{
    dir_ = dir;
}

void Log::SetFormat(const std::string& format)
{
    format_ = format;
}

void Log::SetLogLevel(LOG_LEVEL level)
{
    log_level_ = level;
}

int Log::Initialize()
{
    if (log_on_console_) {
        console_logger_ = spdlog::stdout_color_mt(
            logger_name_, spdlog::color_mode::automatic);
        if (!console_logger_) {
            return E_RETURN_INITLOG_FAIL;
        }
        console_logger_->set_level(
            static_cast<spdlog::level::level_enum>(log_level_));

        console_logger_->set_pattern(format_);
    }

    if (dir_ != "") {
        std::ostringstream oss;
        oss << dir_ << "/" << Utils::GetSystemTime("%Y-%m-%d") << ".log";
        file_logger_ =
            spdlog::rotating_logger_mt(logger_name_ + "_f", oss.str(),
                                       1024 * 1024 * 5,  // 5MB
                                       10);
        if (!file_logger_) {
            return E_RETURN_INITLOG_FAIL;
        }
        file_logger_->set_level(
            static_cast<spdlog::level::level_enum>(log_level_));

        file_logger_->set_pattern(format_);
    }

    return E_RETURN_SUCCESS;
}

}  // namespace edu