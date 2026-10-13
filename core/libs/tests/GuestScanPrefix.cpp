#include "prx/libc/include/general/VabiMacros.hpp"
#include <cstdint>
#include <cstdio>
#include <cstring>

extern "C" int APS5_VABI sscanf_nid_postfix(const char*, const char*, ...);

static int failures = 0;

#define Require(condition) \
    do { \
        if (!(condition)) { \
            std::fprintf(stderr, "line %d: %s\n", __LINE__, #condition); \
            ++failures; \
        } \
    } while (0)

int main() {
    unsigned value = 7;
    int length = -1;
    char rest[16] = {};

    Require(sscanf_nid_postfix("0xg", "%x%s", &value, rest) == 2 && value == 0 && std::strcmp(rest, "xg") == 0);
    value = 7;
    Require(sscanf_nid_postfix("0Xz", "%X%n", &value, &length) == 1 && value == 0 && length == 1);
    int signedValue = 7;
    Require(sscanf_nid_postfix("-0xg", "%i%s", &signedValue, rest) == 2 && signedValue == 0 && std::strcmp(rest, "xg") == 0);
    Require(sscanf_nid_postfix("  +0x", "%i%n", &signedValue, &length) == 1 && signedValue == 0 && length == 4);
    void* pointer = &value;
    Require(sscanf_nid_postfix("0xg", "%p%s", &pointer, rest) == 2 && pointer == nullptr && std::strcmp(rest, "xg") == 0);
    value = 7;
    Require(sscanf_nid_postfix("0x1f", "%x%n", &value, &length) == 1 && value == 0x1f && length == 4);
    Require(sscanf_nid_postfix("0x1", "%2x%s", &value, rest) == 2 && value == 0 && std::strcmp(rest, "x1") == 0);
    Require(sscanf_nid_postfix("0x", "%x%s", &value, rest) == 2 && value == 0 && std::strcmp(rest, "x") == 0);
    Require(sscanf_nid_postfix("0xg", "%*x%s", rest) == 1 && std::strcmp(rest, "xg") == 0);
    Require(sscanf_nid_postfix("0xg", "%*x%n", &length) == 0 && length == 1);
    Require(sscanf_nid_postfix("g", "%x", &value) == 0);
    Require(sscanf_nid_postfix("", "%x", &value) == -1);

    float single = 7.f;
    double twice = 7.;
    Require(sscanf_nid_postfix("1ex", "%f%s", &single, rest) == 2 && single == 1.f && std::strcmp(rest, "ex") == 0);
    Require(sscanf_nid_postfix("1e+x", "%lf%s", &twice, rest) == 2 && twice == 1. && std::strcmp(rest, "e+x") == 0);
    Require(sscanf_nid_postfix("2.5E-", "%lg%n", &twice, &length) == 1 && twice == 2.5 && length == 3);
    Require(sscanf_nid_postfix("1e5x", "%lf%s", &twice, rest) == 2 && twice == 100000. && std::strcmp(rest, "x") == 0);
    Require(sscanf_nid_postfix("1e", "%f%s", &single, rest) == 2 && single == 1.f && std::strcmp(rest, "e") == 0);
    Require(sscanf_nid_postfix("0x1p", "%la%s", &twice, rest) == 2 && twice == 1. && std::strcmp(rest, "p") == 0);
    Require(sscanf_nid_postfix("0x1e", "%la%n", &twice, &length) == 1 && twice == 30. && length == 4);
    Require(sscanf_nid_postfix("1e", "%*f%n", &length) == 0 && length == 1);

    int first = 7;
    int second = 7;
    int before = -1;
    length = -1;
    Require(sscanf_nid_postfix("5 0xg 9", "%d %x %d%n", &first, &value, &second, &length) == 2 && first == 5 && value == 0 && second == 7 && length == -1);
    Require(sscanf_nid_postfix("0xz7", "%x%*c%*c%d%n", &value, &second, &length) == 2 && second == 7 && length == 4);
    length = -1;
    Require(sscanf_nid_postfix("ab 0xg", "%n%2c %x%n", &before, rest, &value, &length) == 2 && before == 0 && length == 4);
    short half = -1;
    signed char tiny = -1;
    Require(sscanf_nid_postfix("0xg", "%x%hn%hhn", &value, &half, &tiny) == 1 && half == 1 && tiny == 1);
    std::int64_t big = -1;
    Require(sscanf_nid_postfix("1ex", "%f%ln", &single, &big) == 1 && big == 1);
    return failures == 0 ? 0 : 1;
}
