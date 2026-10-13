#include "prx/libc/include/general/VabiMacros.hpp"
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

extern "C" {
int APS5_VABI open_nid_postfix(const char*, int, int);
int APS5_VABI close_nid_postfix(int);
std::int64_t APS5_VABI write_nid_postfix(int, const void*, std::uint64_t);
std::int64_t APS5_VABI pread_nid_postfix(int, void*, std::uint64_t, std::int64_t);
std::int64_t APS5_VABI lseek_nid_postfix(int, std::int64_t, int);
int APS5_VABI ftruncate_nid_postfix(int, std::int64_t);
int APS5_VABI truncate_nid_postfix(const char*, std::int64_t);
int APS5_VABI unlink_nid_postfix(const char*);
int* APS5_VABI __error_nid_postfix();
}

namespace {

constexpr int ReadOnly = 0;
constexpr int ReadWrite = 2;
constexpr int Create = 0x200;
constexpr std::int64_t Length = 64ll * 1024 * 1024;
constexpr std::int64_t Chunk = 1024 * 1024;
constexpr long long FastEnoughMilliseconds = 50;
constexpr char FileName[] = "anyps5_truncate_extend_file";
constexpr char BaselineName[] = "anyps5_truncate_extend_baseline";

void Cleanup() {
    unlink_nid_postfix(FileName);
    unlink_nid_postfix(BaselineName);
}

void Require(bool condition, const char* what) {
    if (condition) return;
    std::fprintf(stderr, "%s (errno %d)\n", what, *__error_nid_postfix());
    Cleanup();
    std::abort();
}

long long Milliseconds(std::chrono::steady_clock::time_point start) {
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count();
}

long long WriteZerosMilliseconds() {
    static char zeros[Chunk];
    const int descriptor = open_nid_postfix(BaselineName, Create | ReadWrite, 0644);
    Require(descriptor >= 0, "baseline open failed");
    const auto start = std::chrono::steady_clock::now();
    for (std::int64_t written = 0; written < Length; written += Chunk)
        Require(write_nid_postfix(descriptor, zeros, Chunk) == Chunk, "baseline write failed");
    const long long elapsed = Milliseconds(start);
    Require(close_nid_postfix(descriptor) == 0, "baseline close failed");
    return elapsed;
}

}

int main() {
    Cleanup();
    const long long baseline = WriteZerosMilliseconds();
    const int descriptor = open_nid_postfix(FileName, Create | ReadWrite, 0644);
    Require(descriptor >= 0, "open failed");
    Require(write_nid_postfix(descriptor, "head", 4) == 4, "write failed");

    const auto start = std::chrono::steady_clock::now();
    Require(ftruncate_nid_postfix(descriptor, Length) == 0, "ftruncate to 64 MiB failed");
    const long long elapsed = Milliseconds(start);
    std::printf("extending by 64 MiB took %lld ms, writing 64 MiB of zeros took %lld ms\n", elapsed, baseline);
    Require(elapsed * 10 < baseline || (elapsed < FastEnoughMilliseconds && elapsed * 4 < baseline),
        "extending a file must not write its new length as zero bytes: it took as long as writing the zeros");

    Require(lseek_nid_postfix(descriptor, 0, 2) == Length, "the extended file has the wrong size");
    Require(lseek_nid_postfix(descriptor, 0, 1) == Length, "ftruncate must not move the file offset");
    char head[4] = {};
    char tail[8] = {1, 1, 1, 1, 1, 1, 1, 1};
    Require(pread_nid_postfix(descriptor, head, 4, 0) == 4 && std::memcmp(head, "head", 4) == 0, "extending must keep the existing bytes");
    Require(pread_nid_postfix(descriptor, tail, 8, Length - 8) == 8 && tail[0] == 0 && tail[7] == 0, "the extension must read as zero bytes");

    Require(ftruncate_nid_postfix(descriptor, 2) == 0, "shrinking failed");
    Require(lseek_nid_postfix(descriptor, 0, 2) == 2, "the shrunk file has the wrong size");
    Require(ftruncate_nid_postfix(descriptor, -1) == -1 && *__error_nid_postfix() == 22, "a negative length must fail with EINVAL (22)");
    Require(close_nid_postfix(descriptor) == 0, "close failed");

    const int readOnly = open_nid_postfix(FileName, ReadOnly, 0);
    Require(readOnly >= 0, "reopen failed");
    Require(ftruncate_nid_postfix(readOnly, 8) == -1 && *__error_nid_postfix() == 22, "ftruncate on a read-only descriptor must fail with EINVAL (22)");
    Require(close_nid_postfix(readOnly) == 0, "close failed");

    Require(truncate_nid_postfix(FileName, Length) == 0, "truncate to 64 MiB failed");
    const int check = open_nid_postfix(FileName, ReadOnly, 0);
    Require(check >= 0 && lseek_nid_postfix(check, 0, 2) == Length, "truncate by path left the wrong size");
    Require(close_nid_postfix(check) == 0, "close failed");
    Cleanup();
    return 0;
}
