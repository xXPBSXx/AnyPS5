#include "prx/libc/include/general/VabiMacros.hpp"
#include <cstdlib>

extern "C" {
int APS5_VABI scePlayerSelectionDialogTerminate(void);
}

namespace {

void Require(bool value) { if (!value) std::abort(); }

}

int main() {
    Require(scePlayerSelectionDialogTerminate() == 0);
    Require(scePlayerSelectionDialogTerminate() == 0);
}
