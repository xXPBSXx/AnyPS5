#include "prx/libc/include/general/VabiMacros.hpp"
#include "prx/libkernel/KernelErrors.hpp"
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <exception>

extern "C" {
void* APS5_VABI mmap_nid_postfix(void*, std::size_t, int, int, int, std::int64_t) noexcept;
int APS5_VABI munmap_nid_postfix(void*, std::size_t) noexcept;
int APS5_VABI mprotect_nid_postfix(void*, std::size_t, int) noexcept;
int APS5_VABI sceKernelMprotect(const void*, std::size_t, int);
int APS5_VABI sceKernelMtypeprotect(const void*, std::size_t, int, int);
int* APS5_VABI __error_nid_postfix();
}

namespace {

constexpr std::size_t Page = 0x4000;
constexpr int ErrorInvalid = 22;
constexpr int ProtectReadWrite = 3;

void Require(bool condition, const char* what) {
    if (condition) return;
    std::fprintf(stderr, "%s\n", what);
    std::abort();
}

}

int main() {
    const auto failed = reinterpret_cast<void*>(static_cast<std::uintptr_t>(-1));
    void* raw = mmap_nid_postfix(nullptr, Page * 3, ProtectReadWrite, 0x1002, -1, 0);
    Require(raw != failed, "mmap failed");
    auto* memory = static_cast<unsigned char*>(raw);
    Require(munmap_nid_postfix(memory + Page, Page) == 0, "munmap of the middle page failed");

    int result = 0;
    try {
        result = sceKernelMprotect(memory, Page * 3, 1);
    } catch (const std::exception& error) {
        std::fprintf(stderr, "sceKernelMprotect over an unmapped page threw: %s\n", error.what());
        std::abort();
    }
    Require(result == SCE_KERNEL_ERROR_EINVAL, "sceKernelMprotect over an unmapped page must return EINVAL");

    Require(mprotect_nid_postfix(memory, Page * 3, 1) == -1 && *__error_nid_postfix() == ErrorInvalid, "mprotect over an unmapped page must fail with EINVAL (22)");

    try {
        result = sceKernelMtypeprotect(memory, Page * 3, 0, 1);
    } catch (const std::exception& error) {
        std::fprintf(stderr, "sceKernelMtypeprotect over an unmapped page threw: %s\n", error.what());
        std::abort();
    }
    Require(result == SCE_KERNEL_ERROR_EINVAL, "sceKernelMtypeprotect over an unmapped page must return EINVAL");

    Require(sceKernelMprotect(memory, Page, 1) == 0, "protecting the first page alone must still work");
    Require(sceKernelMprotect(memory + Page * 2, Page, 1) == 0, "protecting the last page alone must still work");

    Require(munmap_nid_postfix(memory, Page) == 0 && munmap_nid_postfix(memory + Page * 2, Page) == 0, "munmap of the remaining pages failed");
    return 0;
}
