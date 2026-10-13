#ifndef CORE_LIBS_PRX_LIBC_INCLUDE_WINDOWSSCANNING_HPP
#define CORE_LIBS_PRX_LIBC_INCLUDE_WINDOWSSCANNING_HPP

#include "WindowsFormatting.hpp"
#include "General.hpp"
#include <algorithm>
#include <cctype>
#include <cerrno>
#include <cstring>
#include <deque>

namespace LibcDetail {

inline size_t ScanOverreach_nid_no_patch(char conversion, const char* begin, size_t length) {
    const char* end = begin + length;
    while (begin < end && std::isspace(static_cast<unsigned char>(*begin))) ++begin;
    const char* digits = begin;
    if (digits < end && (*digits == '+' || *digits == '-')) ++digits;
    if (std::strchr("xXip", conversion)) {
        const bool prefixOnly = end - digits == 2 && digits[0] == '0' && (digits[1] == 'x' || digits[1] == 'X');
        return prefixOnly ? 1 : 0;
    }
    const bool hex = end - digits >= 2 && digits[0] == '0' && (digits[1] == 'x' || digits[1] == 'X');
    const char* tail = end;
    if (tail > digits && (tail[-1] == '+' || tail[-1] == '-')) --tail;
    if (tail - 1 > digits && std::tolower(static_cast<unsigned char>(tail[-1])) == (hex ? 'p' : 'e')) return static_cast<size_t>(end - (tail - 1));
    return 0;
}

struct ScanSegment {
    std::string format;
    std::vector<void*> pointers;
    int* before = nullptr;
    int* after = nullptr;
    char conversion = 0;
    size_t suppressedFirst = 0;
    size_t suppressedLast = 0;
};

struct ScanCount {
    void* destination;
    size_t size;
    int* slot;
    size_t segment;
};

template <typename TScanner>
inline int ScanWindowsArguments_nid_no_patch(const char* format, const void* source, TScanner scan) {
    if (!format || !source) { errno = 22; return EOF; }
    std::vector<ScanSegment> segments(1);
    std::deque<int> marks;
    std::vector<ScanCount> counts;
    std::vector<int> suppressedEnds(static_cast<size_t>(std::count(format, format + std::strlen(format), '%')), -1);
    size_t suppressedCount = 0;
    FormatArguments args(source);
    while (*format) {
        const char value = *format++;
        segments.back().format += value;
        if (value != '%') continue;
        if (*format == '%') { segments.back().format += *format++; continue; }
        const size_t specification = segments.back().format.size() - 1;
        const bool suppressed = *format == '*';
        if (suppressed) segments.back().format += *format++;
        while (*format >= '0' && *format <= '9') segments.back().format += *format++;
        std::string length;
        if (*format && std::strchr("hljztL", *format)) {
            length += *format++;
            if ((length == "h" && *format == 'h') || (length == "l" && *format == 'l')) length += *format++;
        }
        const char conversion = *format;
        if (!conversion) { errno = 22; return EOF; }
        if (!std::strchr("diouxXaAeEfFgGcspn[", conversion)) {
            NotImplemented_nid_no_patch("vsscanf format conversion");
            return EOF;
        }
        ++format;
        const bool resumes = TScanner::Resumable && std::strchr("xXipaAeEfFgG", conversion);
        if (resumes) {
            segments.back().format.insert(specification, "%n");
            marks.push_back(-1);
            segments.back().pointers.push_back(&marks.back());
            segments.back().before = &marks.back();
        }
        if (conversion == 'n' && !suppressed) {
            segments.back().format.resize(specification);
            segments.back().format += "%n";
            marks.push_back(-1);
            segments.back().pointers.push_back(&marks.back());
            const size_t size = length == "hh" ? 1 : length == "h" ? 2 : length.empty() ? 4 : 8;
            counts.push_back({args.Next<void*>(), size, &marks.back(), segments.size() - 1});
            continue;
        }
        if (std::strchr("diouxXn", conversion) && (length == "l" || length == "j" || length == "z" || length == "t"))
            segments.back().format += "ll";
        else segments.back().format += length;
        segments.back().format += conversion;
        if (conversion == '[') {
            if (*format == '^') segments.back().format += *format++;
            if (*format == ']') segments.back().format += *format++;
            while (*format && *format != ']') segments.back().format += *format++;
            if (*format != ']') { errno = 22; return EOF; }
            segments.back().format += *format++;
        }
        if (!suppressed) segments.back().pointers.push_back(args.Next<void*>());
        else if (conversion != 'n') {
            segments.back().format += "%n";
            segments.back().pointers.push_back(&suppressedEnds[suppressedCount++]);
            segments.back().suppressedLast = suppressedCount;
        }
        if (resumes) {
            segments.back().format += "%n";
            marks.push_back(-1);
            segments.back().pointers.push_back(&marks.back());
            segments.back().after = &marks.back();
            segments.back().conversion = conversion;
            ScanSegment next;
            next.suppressedFirst = next.suppressedLast = suppressedCount;
            segments.push_back(std::move(next));
        }
    }
    int assigned = 0;
    bool performed = false;
    size_t offset = 0;
    for (size_t index = 0; index < segments.size(); ++index) {
        auto& segment = segments[index];
        if (segment.format.empty()) continue;
        const int result = scan(segment.format.c_str(), reinterpret_cast<char*>(segment.pointers.data()));
        for (const auto& count : counts) {
            if (count.segment != index || *count.slot < 0) continue;
            const long long total = static_cast<long long>(offset) + *count.slot;
            std::memcpy(count.destination, &total, count.size);
        }
        if (result != EOF) assigned += result;
        performed = performed || result != EOF ||
            std::any_of(suppressedEnds.begin() + segment.suppressedFirst, suppressedEnds.begin() + segment.suppressedLast, [](int end) { return end >= 0; });
        if (!segment.after || *segment.after < 0) break;
        const size_t consumed = static_cast<size_t>(*segment.after) - static_cast<size_t>(*segment.before);
        const size_t kept = static_cast<size_t>(*segment.after) - ScanOverreach_nid_no_patch(segment.conversion, scan.Cursor() + *segment.before, consumed);
        scan.Advance(kept);
        offset += kept;
    }
    return performed ? assigned : EOF;
}

struct StringScanner {
    static constexpr bool Resumable = true;
    const char* cursor;
    int operator()(const char* translated, char* pointers) const { return std::vsscanf(cursor, translated, pointers); }
    const char* Cursor() const { return cursor; }
    void Advance(size_t count) { cursor += count; }
};

struct StreamScanner {
    static constexpr bool Resumable = false;
    std::FILE* stream;
    int operator()(const char* translated, char* pointers) const { return std::vfscanf(stream, translated, pointers); }
    const char* Cursor() const { return nullptr; }
    void Advance(size_t) {}
};

inline int ScanWindows(const char* input, const char* format, const void* source) {
    if (!input) { errno = 22; return EOF; }
    return ScanWindowsArguments_nid_no_patch(format, source, StringScanner{input});
}

inline int ScanFileWindows_nid_no_patch(std::FILE* stream, const char* format, const void* source) {
    if (!stream) { errno = 22; return EOF; }
    return ScanWindowsArguments_nid_no_patch(format, source, StreamScanner{stream});
}

}

#endif
