#include "prx/libc/include/general/VabiMacros.hpp"

#include <cstdio>
#include <stdexcept>
#include <string>

extern "C" {
int APS5_VABI sceAcmBatchJobPriority();
int APS5_VABI sceNpEntitlementAccessGetGameTrialsFlag();
int APS5_VABI scePadVrControllerGetTriggerEffectStates();
int APS5_VABI sceRazorCpuSync();
int APS5_VABI sceSharePlayProhibitController();
int APS5_VABI sceVideoOutUnknown_q_vbO859Tw();
}

namespace {

struct Export {
    const char* name;
    int (APS5_VABI *function)();
};

const Export exports[] = {
    {"sceAcmBatchJobPriority", sceAcmBatchJobPriority},
    {"sceNpEntitlementAccessGetGameTrialsFlag", sceNpEntitlementAccessGetGameTrialsFlag},
    {"scePadVrControllerGetTriggerEffectStates", scePadVrControllerGetTriggerEffectStates},
    {"sceRazorCpuSync", sceRazorCpuSync},
    {"sceSharePlayProhibitController", sceSharePlayProhibitController},
    {"sceVideoOutUnknown_q_vbO859Tw", sceVideoOutUnknown_q_vbO859Tw},
};

bool ReportsNotImplemented(const Export& entry) {
    try {
        entry.function();
    } catch (const std::runtime_error& error) {
        if (std::string(error.what()) == std::string(entry.name) + " not implemented") return true;
        std::fprintf(stderr, "%s threw: %s\n", entry.name, error.what());
        return false;
    }
    std::fprintf(stderr, "%s returned instead of reporting that it is not implemented\n", entry.name);
    return false;
}

}

int main() {
    bool correct = true;
    for (const auto& entry : exports) correct &= ReportsNotImplemented(entry);
    return correct ? 0 : 1;
}
