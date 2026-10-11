#include "prx/libc/include/general/VabiMacros.hpp"

#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <stdexcept>

struct GuestKevent {
    std::uintptr_t ident;
    std::int16_t filter;
    std::uint16_t flags;
    std::uint32_t fflags;
    std::intptr_t data;
    void* udata;
};

struct GuestTimespec {
    std::int64_t seconds;
    std::int64_t nanoseconds;
};

extern "C" {
int APS5_VABI kqueue_nid_postfix();
int APS5_VABI kevent_nid_postfix(int, const GuestKevent*, int, GuestKevent*, int, const GuestTimespec*);
int* APS5_VABI __error_nid_postfix();
}

static void Require(bool value, int line) {
    if (!value) {
        std::fprintf(stderr, "kqueue check failed at line %d\n", line);
        std::abort();
    }
}
#define CHECK(value) Require((value), __LINE__)

int main() {
    constexpr std::int16_t vnode = -4;
    constexpr std::uint16_t add = 0x1, remove = 0x2, enable = 0x4, clear = 0x20;
    constexpr int badDescriptor = 9;
    constexpr int noEntry = 2;
    constexpr int invalid = 22;
    const int queue = kqueue_nid_postfix();
    CHECK(queue >= 0);

    const GuestKevent watch{static_cast<std::uintptr_t>(queue), vnode, add | enable | clear, 0x37, 0, nullptr};
    CHECK(kevent_nid_postfix(queue, &watch, 1, nullptr, 0, nullptr) == 0);

    GuestKevent event{};
    const GuestTimespec brief{0, 30000000};
    const auto start = std::chrono::steady_clock::now();
    CHECK(kevent_nid_postfix(queue, nullptr, 0, &event, 1, &brief) == 0);
    CHECK(std::chrono::steady_clock::now() - start >= std::chrono::milliseconds(25));

    const GuestKevent drop{static_cast<std::uintptr_t>(queue), vnode, remove, 0, 0, nullptr};
    CHECK(kevent_nid_postfix(queue, &drop, 1, nullptr, 0, nullptr) == 0);
    CHECK(kevent_nid_postfix(queue, &drop, 1, nullptr, 0, nullptr) == -1 && *__error_nid_postfix() == noEntry);

    CHECK(kevent_nid_postfix(queue + 1000, nullptr, 0, nullptr, 0, nullptr) == -1 && *__error_nid_postfix() == badDescriptor);
    const GuestKevent closed{static_cast<std::uintptr_t>(queue + 1000), vnode, add, 0, 0, nullptr};
    CHECK(kevent_nid_postfix(queue, &closed, 1, nullptr, 0, nullptr) == -1 && *__error_nid_postfix() == badDescriptor);

    const GuestTimespec badTimeout{0, 1000000000};
    CHECK(kevent_nid_postfix(queue, nullptr, 0, &event, 1, &badTimeout) == -1 && *__error_nid_postfix() == invalid);
    CHECK(kevent_nid_postfix(queue, nullptr, -1, nullptr, 0, nullptr) == -1 && *__error_nid_postfix() == invalid);

    const GuestKevent socketRead{static_cast<std::uintptr_t>(queue), -1, add, 0, 0, nullptr};
    bool threw = false;
    try {
        kevent_nid_postfix(queue, &socketRead, 1, nullptr, 0, nullptr);
    } catch (const std::runtime_error&) {
        threw = true;
    }
    CHECK(threw);
    return 0;
}
