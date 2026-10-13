#include "prx/libc/include/general/VabiMacros.hpp"
#include <cstdint>
#include <cstdio>
#include <cstdlib>

extern "C" {
int APS5_VABI socket_nid_postfix(int, int, int);
int APS5_VABI close_nid_postfix(int);
int APS5_VABI setsockopt_nid_postfix(int, int, int, const void*, std::uint32_t);
int APS5_VABI getsockopt_nid_postfix(int, int, int, void*, std::uint32_t*);
int APS5_VABI fcntl_nid_postfix(int, int, ...);
int* APS5_VABI __error_nid_postfix();
}

namespace {

constexpr int Stream = 1;
constexpr int SolSocket = 0xffff;
constexpr int SoNoSigpipe = 0x800;
constexpr int SoNonBlocking = 0x1200;
constexpr int GetStatusFlags = 3;
constexpr int NonBlockingStatus = 4;
constexpr int ErrorInvalid = 22;

void Require(bool condition, const char* what) {
    if (condition) return;
    std::fprintf(stderr, "%s (errno %d)\n", what, *__error_nid_postfix());
    std::abort();
}

int GetInt(int descriptor, int option) {
    int value = -12345;
    std::uint32_t size = sizeof(value);
    Require(getsockopt_nid_postfix(descriptor, SolSocket, option, &value, &size) == 0 && size == sizeof(value), "getsockopt failed");
    return value;
}

void SetInt(int descriptor, int option, int value) {
    Require(setsockopt_nid_postfix(descriptor, SolSocket, option, &value, sizeof(value)) == 0, "setsockopt failed");
}

}

int main() {
    const int stream = socket_nid_postfix(2, Stream, 0);
    Require(stream >= 0, "socket failed");

    Require(GetInt(stream, SoNoSigpipe) == 0, "SO_NOSIGPIPE must start cleared");
    SetInt(stream, SoNoSigpipe, 1);
    Require(GetInt(stream, SoNoSigpipe) == SoNoSigpipe, "SO_NOSIGPIPE must read back as its own bit");
    SetInt(stream, SoNoSigpipe, 0);
    Require(GetInt(stream, SoNoSigpipe) == 0, "SO_NOSIGPIPE must clear");
    const short shortValue = 1;
    Require(setsockopt_nid_postfix(stream, SolSocket, SoNoSigpipe, &shortValue, sizeof(shortValue)) == -1 && *__error_nid_postfix() == ErrorInvalid, "SO_NOSIGPIPE with a value shorter than an int must fail with EINVAL (22)");

    Require(GetInt(stream, SoNonBlocking) == 0, "the sceNet non-blocking option must start cleared");
    Require((fcntl_nid_postfix(stream, GetStatusFlags) & NonBlockingStatus) == 0, "a new socket is blocking");
    SetInt(stream, SoNonBlocking, 1);
    Require(GetInt(stream, SoNonBlocking) == 1, "the non-blocking option must read back");
    Require((fcntl_nid_postfix(stream, GetStatusFlags) & NonBlockingStatus) != 0, "setting the option 0x1200 must make the socket non-blocking for fcntl too");
    SetInt(stream, SoNonBlocking, 0);
    Require(GetInt(stream, SoNonBlocking) == 0 && (fcntl_nid_postfix(stream, GetStatusFlags) & NonBlockingStatus) == 0, "clearing the option must make the socket blocking again");
    Require(setsockopt_nid_postfix(stream, SolSocket, SoNonBlocking, &shortValue, sizeof(shortValue)) == -1 && *__error_nid_postfix() == ErrorInvalid, "option 0x1200 with a value shorter than an int must fail with EINVAL (22)");

    Require(close_nid_postfix(stream) == 0, "close failed");
    return 0;
}
