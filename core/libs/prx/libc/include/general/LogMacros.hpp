#ifndef CORE_LIBS_PRX_LIBC_INCLUDE_GENERAL_LOGMACROS_HPP
#define CORE_LIBS_PRX_LIBC_INCLUDE_GENERAL_LOGMACROS_HPP

#include <chrono>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <string>

inline bool LogTimestampsEnabled() {
    static const bool enabled = [] {
        const char* value = std::getenv("APS5_LOG_TIMESTAMPS");
        return value != nullptr && *value != 0 && std::strcmp(value, "0") != 0;
    }();
    return enabled;
}

inline int FormatLogTimestamp(char* buffer, std::size_t size, const std::tm& local, int milliseconds) {
    return std::snprintf(buffer, size, "[%02d:%02d:%02d.%03d] ", local.tm_hour, local.tm_min, local.tm_sec, milliseconds);
}

inline std::string CurrentLogTimestamp() {
    const auto now = std::chrono::system_clock::now();
    const auto seconds = std::chrono::system_clock::to_time_t(now);
    const auto milliseconds = static_cast<int>(std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count() % 1000);
    std::tm local{};
#ifdef _WIN32
    localtime_s(&local, &seconds);
#else
    localtime_r(&seconds, &local);
#endif
    char buffer[32];
    const int length = FormatLogTimestamp(buffer, sizeof(buffer), local, milliseconds);
    return std::string(buffer, static_cast<std::size_t>(length));
}

#ifdef __MINGW_PRINTF_FORMAT
[[gnu::format(__MINGW_PRINTF_FORMAT, 2, 3)]]
#else
[[gnu::format(printf, 2, 3)]]
#endif
inline int WriteLogLine(std::FILE* stream, const char* format, ...) {
    char small[1024];
    std::va_list arguments;
    va_start(arguments, format);
    std::va_list again;
    va_copy(again, arguments);
    const int length = std::vsnprintf(small, sizeof(small), format, arguments);
    va_end(arguments);
    std::string large;
    if (length >= static_cast<int>(sizeof(small))) {
        large.resize(static_cast<std::size_t>(length));
        std::vsnprintf(large.data(), large.size() + 1, format, again);
    }
    va_end(again);
    if (length < 0) return length;
    const char* body = large.empty() ? small : large.data();
    if (LogTimestampsEnabled()) {
        std::string line = CurrentLogTimestamp();
        line.append(body, static_cast<std::size_t>(length));
        std::fwrite(line.data(), 1, line.size(), stream);
        return std::fflush(stream);
    }
    std::fwrite(body, 1, static_cast<std::size_t>(length), stream);
    return std::fflush(stream);
}

inline const char* TrimNidPostfix(const char* func) {
    return func;
    const char* pos = std::strstr(func, "_nid_postfix");
    if (pos != nullptr) {
        static thread_local char buf[256];
        const std::size_t len = static_cast<std::size_t>(pos - func);
        std::memcpy(buf, func, len);
        buf[len] = '\0';
        return buf;
    }
    return func;
}

constexpr const char* BaseName(const char* path) {
    const char* last = path;
    const char* prev = nullptr;
    for (const char* p = path; *p; ++p)
        if (*p == '/' || *p == '\\') {
            prev = last;
            last = p + 1;
        }
    return (prev && *prev) ? prev : last;
}

#define _APS5_FILE_ BaseName(__FILE__)

#define _APS5_LOG_IMPL(stream, fmt, ...) \
WriteLogLine(stream, "[%s:%d %s] " fmt "\n", _APS5_FILE_, __LINE__, TrimNidPostfix(__func__), __VA_ARGS__)

#define _APS5_LOG_IMPL_NF(stream, fmt) \
WriteLogLine(stream, "[%s:%d %s] " fmt "\n", _APS5_FILE_, __LINE__, TrimNidPostfix(__func__))

#define APS5_LOG_OUT(fmt, ...) _APS5_LOG_IMPL(stdout, fmt, __VA_ARGS__)
#define APS5_LOG_ERR(fmt, ...) _APS5_LOG_IMPL(stderr, fmt, __VA_ARGS__)
#define APS5_LOG_CHARS_OUT(fmt) _APS5_LOG_IMPL_NF(stdout, fmt)
#define APS5_LOG_CHARS_ERR(fmt) _APS5_LOG_IMPL_NF(stderr, fmt)

#define APS5_LOG_OUT_IF(enabled, fmt, ...) \
do { \
if (enabled) { \
_APS5_LOG_IMPL(stdout, fmt, __VA_ARGS__); \
} \
} while (0)

#define APS5_LOG_CHARS_OUT_IF(enabled, fmt) \
do { \
if (enabled) { \
_APS5_LOG_IMPL_NF(stdout, fmt); \
} \
} while (0)

#define APS5_LOG_TIMING(fmt, ...) APS5_LOG_OUT_IF(APS5_ENABLE_TIMING_LOG, fmt, __VA_ARGS__)
#define APS5_LOG_CHARS_OUT_TIMING(fmt) APS5_LOG_CHARS_OUT_IF(APS5_ENABLE_TIMING_LOG, fmt)

#ifdef APS5_DEBUG_LOG
#define APS5_LOG_OUT_DEBUG(fmt, ...) _APS5_LOG_IMPL(stdout, fmt, __VA_ARGS__)
#define APS5_LOG_CHARS_OUT_DEBUG(fmt) _APS5_LOG_IMPL_NF(stdout, fmt)
#else
#define APS5_LOG_OUT_DEBUG(fmt, ...) ((void)0)
#define APS5_LOG_CHARS_OUT_DEBUG(fmt) ((void)0)
#endif

#endif
