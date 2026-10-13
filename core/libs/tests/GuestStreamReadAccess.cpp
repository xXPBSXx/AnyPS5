#include "prx/libc/include/general/VabiMacros.hpp"
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <initializer_list>

struct FileStream;

extern "C" {
FileStream* APS5_VABI fopen_nid_postfix(const char*, const char*);
int APS5_VABI fclose_nid_postfix(FileStream*);
int APS5_VABI fgetc_nid_postfix(FileStream*);
int APS5_VABI getc_unlocked_nid_postfix(FileStream*);
char* APS5_VABI fgets_nid_postfix(char*, int, FileStream*);
std::size_t APS5_VABI fread_nid_postfix(void*, std::size_t, std::size_t, FileStream*);
int APS5_VABI fscanf_nid_postfix(FileStream*, const char*, ...);
int APS5_VABI fputc_nid_postfix(int, FileStream*);
int APS5_VABI ferror_nid_postfix(FileStream*);
void APS5_VABI clearerr_nid_postfix(FileStream*);
int* APS5_VABI __error_nid_postfix();
extern FileStream* __stdoutp_nid_postfix;
}

static int failures = 0;

#define Require(condition) \
    do { \
        if (!(condition)) { \
            std::fprintf(stderr, "line %d: %s\n", __LINE__, #condition); \
            ++failures; \
        } \
    } while (0)

#define ERR (*__error_nid_postfix())

static void CheckUnreadable(FileStream* stream) {
    char buffer[8];
    int number = 0;
    ERR = 0;
    Require(fgetc_nid_postfix(stream) == EOF && ERR == 9 && ferror_nid_postfix(stream));
    clearerr_nid_postfix(stream);
    ERR = 0;
    Require(getc_unlocked_nid_postfix(stream) == EOF && ERR == 9 && ferror_nid_postfix(stream));
    clearerr_nid_postfix(stream);
    ERR = 0;
    Require(fgets_nid_postfix(buffer, 8, stream) == nullptr && ERR == 9 && ferror_nid_postfix(stream));
    clearerr_nid_postfix(stream);
    ERR = 0;
    Require(fread_nid_postfix(buffer, 1, 4, stream) == 0 && ERR == 9 && ferror_nid_postfix(stream));
    clearerr_nid_postfix(stream);
    ERR = 0;
    Require(fscanf_nid_postfix(stream, "%d", &number) == EOF && ERR == 9 && ferror_nid_postfix(stream));
    clearerr_nid_postfix(stream);
    ERR = 0;
    Require(fread_nid_postfix(buffer, 0, 4, stream) == 0 && ERR == 0 && !ferror_nid_postfix(stream));
}

int main() {
    for (const char* mode : {"w", "wb", "a"}) {
        FileStream* stream = fopen_nid_postfix("anyps5_stream_read_access.txt", mode);
        Require(stream != nullptr);
        if (!stream) continue;
        CheckUnreadable(stream);
        Require(fputc_nid_postfix('x', stream) == 'x');
        Require(fclose_nid_postfix(stream) == 0);
    }
    CheckUnreadable(__stdoutp_nid_postfix);

    FileStream* update = fopen_nid_postfix("anyps5_stream_read_access.txt", "r+");
    Require(update != nullptr);
    if (update) {
        char buffer[8];
        ERR = 0;
        Require(fgetc_nid_postfix(update) == 'x' && ERR == 0);
        Require(fread_nid_postfix(buffer, 1, 4, update) > 0 && !ferror_nid_postfix(update));
        Require(fclose_nid_postfix(update) == 0);
    }
    FileStream* reader = fopen_nid_postfix("anyps5_stream_read_access.txt", "r");
    Require(reader != nullptr);
    if (reader) {
        Require(fgetc_nid_postfix(reader) == 'x');
        Require(fclose_nid_postfix(reader) == 0);
    }
    std::remove("anyps5_stream_read_access.txt");
    return failures == 0 ? 0 : 1;
}
