#include "prx/libc/include/general/VabiMacros.hpp"

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <initializer_list>

extern "C" {
int APS5_VABI getrlimit_nid_postfix(int, void*);
int APS5_VABI getrusage_nid_postfix(int, void*);
int APS5_VABI gettimeofday_nid_postfix(void*, void*);
int APS5_VABI getitimer_nid_postfix(int, void*);
int APS5_VABI setitimer_nid_postfix(int, const void*, void*);
int APS5_VABI sceKernelGettimeofday(void*);
int APS5_VABI sceKernelGettimezone(void*);
int* APS5_VABI __error_nid_postfix();
}

static void Check(bool value, int line) {
    if (!value) {
        std::fprintf(stderr, "output pointer check failed at line %d\n", line);
        std::abort();
    }
}
#define Require(value) Check((value), __LINE__)

static constexpr int GuestEfault = 14;
static constexpr int KernelEfault = static_cast<int>(0x8002000Eu);

static bool Efault(int result) {
    return result == -1 && *__error_nid_postfix() == GuestEfault;
}

int main() {
    alignas(8) unsigned char valid[512]{};
    alignas(8) unsigned char zeroTimer[32]{};
    for (const std::uintptr_t address : {std::uintptr_t{1}, ~std::uintptr_t{0xfff}}) {
        void* bad = reinterpret_cast<void*>(address);
        *__error_nid_postfix() = 0;
        Require(Efault(getrlimit_nid_postfix(2, bad)));
        *__error_nid_postfix() = 0;
        Require(Efault(getrusage_nid_postfix(0, bad)));
        *__error_nid_postfix() = 0;
        Require(Efault(gettimeofday_nid_postfix(bad, nullptr)));
        *__error_nid_postfix() = 0;
        Require(Efault(gettimeofday_nid_postfix(valid, bad)));
        *__error_nid_postfix() = 0;
        Require(Efault(getitimer_nid_postfix(0, bad)));
        *__error_nid_postfix() = 0;
        Require(Efault(setitimer_nid_postfix(0, zeroTimer, bad)));
        *__error_nid_postfix() = 0;
        Require(Efault(setitimer_nid_postfix(0, bad, valid)));
        Require(sceKernelGettimeofday(bad) == KernelEfault);
        Require(sceKernelGettimezone(bad) == KernelEfault);
    }
    Require(getrlimit_nid_postfix(2, valid) == 0);
    Require(getrusage_nid_postfix(0, valid) == 0);
    Require(gettimeofday_nid_postfix(valid, valid + 32) == 0);
    Require(getitimer_nid_postfix(0, valid) == 0);
    Require(setitimer_nid_postfix(0, zeroTimer, valid) == 0);
    Require(sceKernelGettimeofday(valid) == 0);
    Require(sceKernelGettimezone(valid) == 0);
}
