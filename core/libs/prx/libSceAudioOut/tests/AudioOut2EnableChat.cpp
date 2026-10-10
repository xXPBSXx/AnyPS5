#include "prx/libc/include/general/VabiMacros.hpp"

#include <cstdio>
#include <stdexcept>
#include <string>

extern "C" int APS5_VABI sceAudioOut2EnableChat();

int main() {
    try {
        sceAudioOut2EnableChat();
    } catch (const std::runtime_error& error) {
        if (std::string(error.what()) == "sceAudioOut2EnableChat not implemented") return 0;
        std::fprintf(stderr, "sceAudioOut2EnableChat threw: %s\n", error.what());
        return 1;
    }
    std::fprintf(stderr, "sceAudioOut2EnableChat returned instead of reporting that it is not implemented\n");
    return 1;
}
