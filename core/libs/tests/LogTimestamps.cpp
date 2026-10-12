#include "prx/libc/include/general/LogMacros.hpp"
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <string>

static void Require(bool condition, const std::string& message) {
    if (!condition) {
        std::fprintf(stderr, "%s\n", message.c_str());
        std::exit(1);
    }
}

static std::string FormatAt(int hour, int minute, int second, int milliseconds) {
    std::tm local{};
    local.tm_hour = hour;
    local.tm_min = minute;
    local.tm_sec = second;
    char buffer[32];
    const int length = FormatLogTimestamp(buffer, sizeof(buffer), local, milliseconds);
    return std::string(buffer, static_cast<std::size_t>(length));
}

static std::string WrittenLine() {
    std::FILE* file = std::tmpfile();
    Require(file != nullptr, "tmpfile failed");
    WriteLogLine(file, "hello %d\n", 42);
    std::rewind(file);
    char buffer[128] = {};
    const std::size_t count = std::fread(buffer, 1, sizeof(buffer) - 1, file);
    std::fclose(file);
    return std::string(buffer, count);
}

static bool IsTimestamped(const std::string& line) {
    const char* pattern = "[dd:dd:dd.ddd] hello 42\n";
    if (line.size() != std::string(pattern).size()) return false;
    for (std::size_t index = 0; index < line.size(); ++index) {
        if (pattern[index] == 'd') {
            if (line[index] < '0' || line[index] > '9') return false;
        } else if (line[index] != pattern[index]) {
            return false;
        }
    }
    return true;
}

int main(int argc, char** argv) {
    Require(argc == 2, "usage: log_timestamps_tests on|off");
    Require(FormatAt(3, 4, 5, 6) == "[03:04:05.006] ", "the timestamp pads hours, minutes, seconds and milliseconds");
    Require(FormatAt(23, 59, 59, 999) == "[23:59:59.999] ", "the timestamp keeps the largest values");
    Require(FormatAt(0, 0, 0, 0) == "[00:00:00.000] ", "the timestamp keeps midnight");
    const std::string now = CurrentLogTimestamp();
    Require(now.size() == 15 && now[0] == '[' && now[3] == ':' && now[6] == ':' && now[9] == '.' && now[13] == ']' && now[14] == ' ', "the current timestamp has the form [HH:MM:SS.mmm] ");
    const std::string line = WrittenLine();
    const std::string mode = argv[1];
    if (mode == "on") {
        Require(LogTimestampsEnabled(), "APS5_LOG_TIMESTAMPS=1 must enable the timestamp");
        Require(IsTimestamped(line), "a log line must start with the timestamp, got: " + line);
    } else {
        Require(!LogTimestampsEnabled(), "the timestamp must be off for this environment");
        Require(line == "hello 42\n", "a log line must stay unchanged without the switch, got: " + line);
    }
    return 0;
}
