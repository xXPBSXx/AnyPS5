#include "SceTypes.hpp"
#include "prx/libc/include/General.hpp"
#include "prx/libkernel/File/include/File.hpp"

#include <cerrno>
#include <chrono>
#include <cstdint>
#include <fcntl.h>
#include <map>
#include <mutex>
#include <set>
#include <stdexcept>
#include <string>
#include <thread>
#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

extern "C" int* APS5_VABI __error_nid_postfix();

namespace {

constexpr int GUEST_ENOENT = 2;
constexpr int GUEST_ENOMEM = 12;
constexpr int GUEST_EBADF = 9;
constexpr int GUEST_EFAULT = 14;
constexpr int GUEST_EINVAL = 22;
constexpr int GUEST_ENFILE = 23;
constexpr int GUEST_EMFILE = 24;

constexpr std::int16_t EVFILT_VNODE = -4;
constexpr std::uint16_t EV_ADD = 0x0001;
constexpr std::uint16_t EV_DELETE = 0x0002;
constexpr std::uint16_t EV_ENABLE = 0x0004;
constexpr std::uint16_t EV_DISABLE = 0x0008;
constexpr std::uint16_t EV_CLEAR = 0x0020;
constexpr std::uint16_t SupportedFlags = EV_ADD | EV_DELETE | EV_ENABLE | EV_DISABLE | EV_CLEAR;

struct GuestKevent {
    std::uintptr_t ident;
    std::int16_t filter;
    std::uint16_t flags;
    std::uint32_t fflags;
    std::intptr_t data;
    void* udata;
};
static_assert(sizeof(GuestKevent) == 32);

struct GuestTimespec {
    std::int64_t seconds;
    std::int64_t nanoseconds;
};

std::mutex registry;
std::map<int, std::set<std::uintptr_t>> queues;

int Failure(int error) {
    *__error_nid_postfix() = error;
    return -1;
}

}

extern "C" {

int APS5_VABI kqueue_nid_postfix() {
#ifdef _WIN32
    const int descriptor = ::_open("NUL", _O_RDWR | _O_BINARY);
#else
    const int descriptor = ::open("/dev/null", O_RDWR);
#endif
    if (descriptor < 0) return Failure(errno == EMFILE ? GUEST_EMFILE : errno == ENOMEM ? GUEST_ENOMEM : GUEST_ENFILE);
    std::lock_guard lock(registry);
    queues[descriptor].clear();
    return descriptor;
}

int APS5_VABI kevent_nid_postfix(int queue, const GuestKevent* changes, int changeCount, GuestKevent* events, int eventCount, const GuestTimespec* timeout) {
    if (changeCount < 0 || eventCount < 0) return Failure(GUEST_EINVAL);
    if ((changeCount > 0 && changes == nullptr) || (eventCount > 0 && events == nullptr)) return Failure(GUEST_EFAULT);
    if (timeout != nullptr && (timeout->seconds < 0 || timeout->nanoseconds < 0 || timeout->nanoseconds >= 1000000000)) return Failure(GUEST_EINVAL);
    {
        std::lock_guard lock(registry);
        const auto found = queues.find(queue);
        if (found == queues.end() || !DescriptorIsOpen_nid_no_patch(queue)) return Failure(GUEST_EBADF);
        for (int index = 0; index < changeCount; ++index) {
            const auto& change = changes[index];
            if (change.filter != EVFILT_VNODE) throw std::runtime_error("kevent: only the vnode filter is supported, got filter " + std::to_string(change.filter));
            if ((change.flags & ~SupportedFlags) != 0) throw std::runtime_error("kevent: unsupported change flags " + std::to_string(change.flags));
            const auto descriptor = static_cast<int>(change.ident);
            if ((change.flags & EV_DELETE) != 0) {
                if (found->second.erase(change.ident) == 0) return Failure(GUEST_ENOENT);
            } else if ((change.flags & EV_ADD) != 0) {
                if (!DescriptorIsOpen_nid_no_patch(descriptor)) return Failure(GUEST_EBADF);
                found->second.insert(change.ident);
            } else if ((change.flags & (EV_ENABLE | EV_DISABLE)) != 0) {
                if (found->second.count(change.ident) == 0) return Failure(GUEST_ENOENT);
            }
        }
    }
    if (eventCount == 0) return 0;
    if (timeout == nullptr) {
        for (;;) std::this_thread::sleep_for(std::chrono::hours(1));
    }
    std::this_thread::sleep_for(std::chrono::seconds(timeout->seconds) + std::chrono::nanoseconds(timeout->nanoseconds));
    return 0;
}

}
