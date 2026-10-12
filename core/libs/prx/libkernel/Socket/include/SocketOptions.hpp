#pragma once
#include <cstdint>

namespace GuestSockets {

struct SocketOptionState {
    std::uint32_t stored = 0;
    int reportedError = 0;
};

int SetSocketLevelOption(std::uintptr_t native, SocketOptionState& state, int option, const void* value, std::uint32_t length);
int GetSocketLevelOption(std::uintptr_t native, SocketOptionState& state, int type, int option, void* value, std::uint32_t* length);

}
