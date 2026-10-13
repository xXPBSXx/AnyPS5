#include "prx/libc/include/general/VabiMacros.hpp"
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <initializer_list>

extern "C" {
int APS5_VABI socket_nid_postfix(int, int, int);
int APS5_VABI setsockopt_nid_postfix(int, int, int, const void*, std::uint32_t);
int APS5_VABI getsockopt_nid_postfix(int, int, int, void*, std::uint32_t*);
int APS5_VABI close_nid_postfix(int);
int* APS5_VABI __error_nid_postfix();
}

namespace {

constexpr int Stream = 1;
constexpr int Datagram = 2;
constexpr int IpProtoTcp = 6;
constexpr int TcpNoDelay = 1;
constexpr int TcpNoPush = 4;
constexpr int TcpNoOptions = 8;
constexpr int TcpKeepInit = 128;
constexpr int TcpKeepIdle = 256;
constexpr int TcpKeepInterval = 512;
constexpr int TcpKeepCount = 1024;
constexpr int TcpMaxSegment = 2;
constexpr int ErrorInvalid = 22;
constexpr int ErrorNoProtocolOption = 42;

void Require(bool condition, const char* what) {
    if (condition) return;
    std::fprintf(stderr, "%s (errno %d)\n", what, *__error_nid_postfix());
    std::abort();
}

int GetTcp(int descriptor, int option) {
    unsigned int value = 0xdeadbeef;
    std::uint32_t size = sizeof(value);
    Require(getsockopt_nid_postfix(descriptor, IpProtoTcp, option, &value, &size) == 0 && size == sizeof(value), "getsockopt at IPPROTO_TCP failed");
    return static_cast<int>(value);
}

void SetTcp(int descriptor, int option, unsigned int value) {
    Require(setsockopt_nid_postfix(descriptor, IpProtoTcp, option, &value, sizeof(value)) == 0, "setsockopt at IPPROTO_TCP failed");
}

bool SetFails(int descriptor, int level, int option, const void* value, std::uint32_t length, int error) {
    return setsockopt_nid_postfix(descriptor, level, option, value, length) == -1 && *__error_nid_postfix() == error;
}

void BooleanOptionsReportTheirFlag() {
    const int stream = socket_nid_postfix(2, Stream, 0);
    Require(stream >= 0, "socket failed");
    Require(GetTcp(stream, TcpNoDelay) == 0, "TCP_NODELAY must start cleared");
    SetTcp(stream, TcpNoDelay, 1);
    Require(GetTcp(stream, TcpNoDelay) == 4, "TCP_NODELAY set must read back as TF_NODELAY (4)");
    SetTcp(stream, TcpNoDelay, 0);
    Require(GetTcp(stream, TcpNoDelay) == 0, "TCP_NODELAY must clear");
    Require(close_nid_postfix(stream) == 0, "close failed");
}

void KeepAliveTimersRoundTrip() {
    const int stream = socket_nid_postfix(2, Stream, 0);
    Require(stream >= 0, "socket failed");
    SetTcp(stream, TcpKeepIdle, 120);
    Require(GetTcp(stream, TcpKeepIdle) == 120, "TCP_KEEPIDLE must round trip");
    SetTcp(stream, TcpKeepInterval, 15);
    Require(GetTcp(stream, TcpKeepInterval) == 15, "TCP_KEEPINTVL must round trip");
    SetTcp(stream, TcpKeepCount, 9);
    Require(GetTcp(stream, TcpKeepCount) == 9, "TCP_KEEPCNT must round trip");
    const unsigned int huge = 0xffffffffu;
    Require(SetFails(stream, IpProtoTcp, TcpKeepIdle, &huge, sizeof(huge), ErrorInvalid), "a keep-alive time above UINT_MAX/hz must fail with EINVAL (22)");
    Require(close_nid_postfix(stream) == 0, "close failed");
}

void RequestsOutsideTheSupportedSetFail() {
    const int stream = socket_nid_postfix(2, Stream, 0);
    Require(stream >= 0, "socket failed");
    const unsigned int one = 1;
    Require(SetFails(stream, IpProtoTcp, TcpNoDelay, &one, 2, ErrorInvalid), "a TCP option shorter than an int must fail with EINVAL (22)");
    Require(SetFails(stream, IpProtoTcp, 0x7fff, &one, sizeof(one), ErrorNoProtocolOption), "an unknown TCP option must fail with ENOPROTOOPT (42)");
    for (const int option : {TcpNoPush, TcpNoOptions, TcpKeepInit}) {
        Require(SetFails(stream, IpProtoTcp, option, &one, sizeof(one), ErrorNoProtocolOption), "an option no title needs must fail with ENOPROTOOPT (42)");
    }
    Require(SetFails(stream, IpProtoTcp, TcpMaxSegment, &one, sizeof(one), ErrorNoProtocolOption), "an unimplemented TCP option must fail with ENOPROTOOPT (42)");
    unsigned int value = 0;
    std::uint32_t size = sizeof(value);
    Require(getsockopt_nid_postfix(stream, IpProtoTcp, TcpMaxSegment, &value, &size) == -1 && *__error_nid_postfix() == ErrorNoProtocolOption, "getsockopt of an unimplemented TCP option must fail with ENOPROTOOPT (42)");
    Require(close_nid_postfix(stream) == 0, "close failed");

    const int datagram = socket_nid_postfix(2, Datagram, 0);
    Require(datagram >= 0, "socket failed");
    Require(SetFails(datagram, IpProtoTcp, TcpNoDelay, &one, sizeof(one), ErrorNoProtocolOption), "IPPROTO_TCP on a datagram socket must fail with ENOPROTOOPT (42)");
    Require(close_nid_postfix(datagram) == 0, "close failed");
}

}

int main() {
    BooleanOptionsReportTheirFlag();
    KeepAliveTimersRoundTrip();
    RequestsOutsideTheSupportedSetFail();
    return 0;
}
