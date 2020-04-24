#include <common/config.h>
#include <common/log.h>
#include <common/utils.h>
#include <push_sdk.h>

#include <sstream>

#include <grpc/grpc.h>
#include <grpc/impl/codegen/log.h>

#define DEFAULT_FORMAT "[%Y-%m-%d %H:%M:%S.%e][%t][%l]%v%$"
#define SDK_LOGGER_NAME "push_sdk"
#define GRPC_LOGGER_NAME "grpc"
#define GRPC_LOG_PREFIX ": {}"
#define GRPC_LOG_PREFIX_DEBUG "[{}:{}]: {}"

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
    if (dir.empty()) {
        return;
    }

    std::string s;
    if (dir != "" && dir.find_last_of('/') == dir.length() - 1) {
        s = dir.substr(0, dir.length() - 1);
    }
    else {
        s = dir;
    }
    dir_ = s + "/" + logger_name_;
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
            return PS_RET_INIT_LOG_FAILED;
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
            return PS_RET_INIT_LOG_FAILED;
        }
        file_logger_->set_level(
            static_cast<spdlog::level::level_enum>(log_level_));

        file_logger_->set_pattern(format_);
    }

    return PS_RET_SUCCESS;
}

}  // namespace edu

std::shared_ptr<edu::Log> _sdk_logger  = nullptr;
std::shared_ptr<edu::Log> _grpc_logger = nullptr;

static void grpc_log_func(gpr_log_func_args* args)
{
#if PUSH_SDK_DEBUG
    switch (args->severity) {
        case GPR_LOG_SEVERITY_DEBUG:
            _grpc_logger->Debug(GRPC_LOG_PREFIX_DEBUG, args->file, args->line,
                                args->message);
            break;
        case GPR_LOG_SEVERITY_INFO:
            _grpc_logger->Info(GRPC_LOG_PREFIX_DEBUG, args->file, args->line,
                               args->message);
            break;
        case GPR_LOG_SEVERITY_ERROR:
            _grpc_logger->Error(GRPC_LOG_PREFIX_DEBUG, args->file, args->line,
                                args->message);
            break;
    }
#else
    switch (args->severity) {
        case GPR_LOG_SEVERITY_DEBUG:
            _grpc_logger->Debug(GRPC_LOG_PREFIX, args->message);
            break;
        case GPR_LOG_SEVERITY_INFO:
            _grpc_logger->Info(GRPC_LOG_PREFIX, args->message);
            break;
        case GPR_LOG_SEVERITY_ERROR:
            _grpc_logger->Error(GRPC_LOG_PREFIX, args->message);
            break;
    }
#endif
}

int init_logger(const std::string& log_dir)
{
    int ret = PS_RET_SUCCESS;

    try {
        // initializing sdk logger
        _sdk_logger = std::make_shared<edu::Log>(SDK_LOGGER_NAME);
        _sdk_logger->LogOnConsole(edu::Config::Instance()->sdk_log_on_console);
        _sdk_logger->SetOutputDir(log_dir);
        _sdk_logger->SetLogLevel(
            edu::Utils::StrToLogLevel(edu::Config::Instance()->sdk_log_level));
        if ((ret = _sdk_logger->Initialize()) != PS_RET_SUCCESS) {
            return ret;
        }

        // initializing grpc logger
        _grpc_logger = std::make_shared<edu::Log>(GRPC_LOGGER_NAME);
        _grpc_logger->LogOnConsole(
            edu::Config::Instance()->grpc_log_on_console);
        _grpc_logger->SetOutputDir(log_dir);
        _grpc_logger->SetLogLevel(
            edu::Utils::StrToLogLevel(edu::Config::Instance()->grpc_log_level));
        if ((ret = _grpc_logger->Initialize()) != PS_RET_SUCCESS) {
            return ret;
        }

        // 让grpc输出最低等级日志,用日志函数进行过滤
        gpr_set_log_verbosity(GPR_LOG_SEVERITY_DEBUG);
        gpr_log_verbosity_init();
        gpr_set_log_function(&grpc_log_func);

        // grpc_tracer_set_enabled("subchannel", 1);
        // grpc_tracer_set_enabled("client_channel_routing", 1);
        // grpc_tracer_set_enabled("client_channel_call", 1);
        // grpc_tracer_set_enabled("connectivity_state", 1);
        // grpc_tracer_set_enabled("call_error", 1);
        // grpc_tracer_set_enabled("pick_first", 1);
        // grpc_tracer_set_enabled("channel", 1);
        // grpc_tracer_set_enabled("op_failure", 1);

        // grpc_tracer_set_enabled("resolver_refcount", 1);
        // grpc_tracer_set_enabled("flowctl", 1);
        // grpc_tracer_set_enabled("list_tracers", 1);
        // grpc_tracer_set_enabled("http2_stream_state", 1);
        // grpc_tracer_set_enabled("bdp_estimator", 1);
        // grpc_tracer_set_enabled("cares_resolver", 1);
        // grpc_tracer_set_enabled("cares_address_sorting", 1);
        // grpc_tracer_set_enabled("round_robin", 1);
    }
    catch (std::exception& e) {
        ret = PS_RET_INIT_LOG_FAILED;
        return ret;
    }
    return ret;
}
