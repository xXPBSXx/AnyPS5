#include "prx/libc/include/general/VabiMacros.hpp"
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <limits>

extern "C" {
int APS5_VABI open_nid_postfix(const char*, int, int);
int APS5_VABI close_nid_postfix(int);
int APS5_VABI unlink_nid_postfix(const char*);
std::int64_t APS5_VABI lseek_nid_postfix(int, std::int64_t, int);
std::int64_t APS5_VABI read_nid_postfix(int, void*, std::uint64_t);
std::int64_t APS5_VABI write_nid_postfix(int, const char*, std::int64_t);
std::int64_t APS5_VABI pread_nid_postfix(int, void*, std::size_t, std::int64_t);
std::int64_t APS5_VABI pwrite_nid_disambig1_nid_postfix(int, const void*, std::size_t, std::int64_t);
std::int64_t APS5_VABI sceKernelRead(int, void*, std::size_t);
std::int64_t APS5_VABI sceKernelWrite(int, const void*, std::size_t);
std::int64_t APS5_VABI sceKernelPread(int, void*, std::size_t, std::int64_t);
std::int64_t APS5_VABI sceKernelPwrite(int, const void*, std::size_t, std::int64_t);
int* APS5_VABI __error_nid_postfix();
}

namespace {

constexpr int ReadWrite = 2;
constexpr int Create = 0x200;
constexpr int ErrorInvalid = 22;
constexpr std::int64_t SceInvalid = static_cast<std::int32_t>(0x80020016u);
constexpr char FileName[] = "anyps5_large_transfer_file";
constexpr char Content[] = "0123456789";
int openDescriptor = -1;

void Require(bool condition, const char* what) {
    if (condition) return;
    const int error = *__error_nid_postfix();
    if (openDescriptor >= 0) close_nid_postfix(openDescriptor);
    unlink_nid_postfix(FileName);
    std::fprintf(stderr, "%s (errno %d)\n", what, error);
    std::abort();
}

bool Fails(std::int64_t result, int error) { return result == -1 && *__error_nid_postfix() == error; }

}

int main() {
    unlink_nid_postfix(FileName);
    const int descriptor = open_nid_postfix(FileName, Create | ReadWrite, 0644);
    openDescriptor = descriptor;
    Require(descriptor >= 0, "open failed");
    Require(write_nid_postfix(descriptor, Content, 10) == 10, "write of the test content failed");
    const std::uint64_t counts[] = {
        std::uint64_t{std::numeric_limits<std::int32_t>::max()} + 1,
        std::uint64_t{std::numeric_limits<std::uint32_t>::max()} + 1,
    };
    char* buffer = static_cast<char*>(std::malloc(counts[1]));
    Require(buffer != nullptr, "the 4 GiB buffer could not be allocated");
    for (const std::uint64_t count : counts) {
        std::memset(buffer, 0, 64);
        Require(lseek_nid_postfix(descriptor, 0, 0) == 0, "lseek failed");
        Require(read_nid_postfix(descriptor, buffer, count) == 10 && std::memcmp(buffer, Content, 10) == 0, "read of a count above the host limit must return the 10 bytes of the file");
        std::memset(buffer, 0, 64);
        Require(pread_nid_postfix(descriptor, buffer, count, 0) == 10 && std::memcmp(buffer, Content, 10) == 0, "pread of a count above the host limit must return the 10 bytes of the file");
        std::memset(buffer, 0, 64);
        Require(lseek_nid_postfix(descriptor, 0, 0) == 0, "lseek failed");
        Require(sceKernelRead(descriptor, buffer, count) == 10 && std::memcmp(buffer, Content, 10) == 0, "sceKernelRead of a count above the host limit must return the 10 bytes of the file");
        std::memset(buffer, 0, 64);
        Require(sceKernelPread(descriptor, buffer, count, 0) == 10 && std::memcmp(buffer, Content, 10) == 0, "sceKernelPread of a count above the host limit must return the 10 bytes of the file");
    }
    std::free(buffer);
    char small[16];
    const std::uint64_t beyond = std::uint64_t{std::numeric_limits<std::int64_t>::max()} + 1;
    Require(Fails(read_nid_postfix(descriptor, small, beyond), ErrorInvalid), "read of a count above SSIZE_MAX must fail with EINVAL (22)");
    Require(Fails(pread_nid_postfix(descriptor, small, beyond, 0), ErrorInvalid), "pread of a count above SSIZE_MAX must fail with EINVAL (22)");
    Require(Fails(pwrite_nid_disambig1_nid_postfix(descriptor, small, beyond, 0), ErrorInvalid), "pwrite of a count above SSIZE_MAX must fail with EINVAL (22)");
    Require(sceKernelRead(descriptor, small, beyond) == SceInvalid, "sceKernelRead of a count above SSIZE_MAX must return EINVAL");
    Require(sceKernelWrite(descriptor, small, beyond) == SceInvalid, "sceKernelWrite of a count above SSIZE_MAX must return EINVAL");
    Require(sceKernelPread(descriptor, small, beyond, 0) == SceInvalid, "sceKernelPread of a count above SSIZE_MAX must return EINVAL");
    Require(sceKernelPwrite(descriptor, small, beyond, 0) == SceInvalid, "sceKernelPwrite of a count above SSIZE_MAX must return EINVAL");
    openDescriptor = -1;
    Require(close_nid_postfix(descriptor) == 0, "close failed");
    unlink_nid_postfix(FileName);
    return 0;
}
