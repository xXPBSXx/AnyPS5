#include "prx/libc/include/general/VabiMacros.hpp"

#include <cstdio>
#include <stdexcept>
#include <string>

extern "C" int APS5_VABI sceHttp2WebSocketSetPingInterval();

int main() {
    try {
        sceHttp2WebSocketSetPingInterval();
    } catch (const std::runtime_error& error) {
        if (std::string(error.what()) == "sceHttp2WebSocketSetPingInterval not implemented") return 0;
        std::fprintf(stderr, "sceHttp2WebSocketSetPingInterval threw: %s\n", error.what());
        return 1;
    }
    std::fprintf(stderr, "sceHttp2WebSocketSetPingInterval returned instead of reporting that it is not implemented\n");
    return 1;
}
