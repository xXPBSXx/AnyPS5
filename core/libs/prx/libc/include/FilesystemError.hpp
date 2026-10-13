#ifndef CORE_LIBS_PRX_LIBC_INCLUDE_FILESYSTEMERROR_HPP
#define CORE_LIBS_PRX_LIBC_INCLUDE_FILESYSTEMERROR_HPP

#include <filesystem>
#include <string>
#include <system_error>
#include "General.hpp"

inline int FilesystemError(const std::error_code& error) {
    if (error == std::errc::no_such_file_or_directory) return 2;
    if (error == std::errc::permission_denied) return 13;
    if (error == std::errc::operation_not_permitted) return 1;
    if (error == std::errc::not_a_directory) return 20;
    if (error == std::errc::is_a_directory) return 21;
    if (error == std::errc::directory_not_empty) return 66;
    if (error == std::errc::device_or_resource_busy) return 16;
    if (error == std::errc::read_only_file_system) return 30;
    if (error == std::errc::filename_too_long) return 63;
    if (error == std::errc::too_many_symbolic_link_levels) return 62;
    if (error == std::errc::not_enough_memory) return 12;
    if (error == std::errc::invalid_argument) return 22;
    if (error == std::errc::cross_device_link) return 18;
    if (error == std::errc::file_exists) return 17;
    if (error == std::errc::no_space_on_device) return 28;
    return 5;
}

inline int ResolvePathError_nid_no_patch(const char* path, std::filesystem::path& resolved) {
    try {
        resolved = ResolvePath_nid_no_patch(path);
        return 0;
    } catch (const std::filesystem::filesystem_error& error) {
        if (error.code() == std::errc::illegal_byte_sequence) return 2;
        return FilesystemError(error.code());
    }
}

inline int RenameDotError_nid_no_patch(const char* path) {
    std::size_t length = std::char_traits<char>::length(path);
    while (length > 1 && path[length - 1] == '/') --length;
    std::size_t start = length;
    while (start > 0 && path[start - 1] != '/') --start;
    if (length - start != 1 || path[start] != '.') return 0;
    const std::string parent = start == 0 ? std::string(".") : std::string(path, start);
    std::filesystem::path resolved;
    if (const int error = ResolvePathError_nid_no_patch(parent.c_str(), resolved)) return error;
    std::error_code status;
    if (!std::filesystem::exists(resolved, status)) return 2;
    return std::filesystem::is_directory(resolved, status) ? 22 : 20;
}

#endif
