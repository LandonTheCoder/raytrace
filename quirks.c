// This contains per-OS functions to handle quirks.

// OS-agnostic includes
// For setlocale()
#include <locale.h>

#ifdef _WIN32
// FormatMessage(), LocalFree(), GetStdHandle(), GetConsoleMode(), SetConsoleMode(), GetLastError()
#include <windows.h>
// int32_t
#include <stdint.h>
// printf(), wprintf()
#include <stdio.h>
// These 2 are used for _setmode and _fileno, not sure which has which.
#include <fcntl.h>
#include <io.h>

// Utility functions

// This converts an integer error to a string error and logs it.
static void print_win_err(const char *func_name, int32_t err) {
    LPTSTR msg_buf, display_buf;
    // FormatMessage(DWORD flags, LPCVOID src, DWORD msg_id, DWORD lang_id,
    //               LPTSTR buf (out), DWORD size, ...)
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                  FORMAT_MESSAGE_FROM_SYSTEM |
                  FORMAT_MESSAGE_IGNORE_INSERTS,
                  NULL,
                  err,
                  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                  (LPTSTR) &msg_buf, 0, NULL);
    // msg_buf is a wchar_t * if UNICODE is defined, else char *.
#ifdef UNICODE
    // %ls means wchar_t *str, %s means char *str
    fwprintf(stderr, L"%s failed with error %i: %ls\n", func_name, err, msg_buf);
#else
    fprintf(stderr, "%s failed with error %i: %s\n", func_name, err, msg_buf);
#endif
    LocalFree(msg_buf);
}

// The basic Windows version
struct winver {
    int32_t major;
    int32_t minor;
    int32_t build;
};

// For system checks
static struct winver get_win_version(void) {
    struct winver retval;
    /* Returns packed int32 containing:
     *   - build in top 16 bits
     *   - minor in top 8 bits of bottom 16 bits
     *   - major in bottom 8 bits of bottom 16 bits
     */
    uint32_t winver = GetVersion();
    // Low byte of the bottom 16 bits
    retval.major = winver & 0xFF;
    // High byte of the bottom 16 bits
    retval.minor = (winver & 0xFFFF) >> 8;
    // Top 16 bits (Microsoft says I need this check here?)
    if (winver < 0x80000000)
        retval.build = (winver >> 16);
    else
        retval.build = 0;
    return retval;
}

// Exported functions

// Returns 0 if fully successful (UTF-8 codepage), real codepage otherwise
int ensure_locale(void) {
    /* Determine whether UTF-8 codepage is supported. I have to explicitly opt
     * into the UTF-8 codepage, which doesn't work on builds before 1903 or so.
     * If not supported (or broken), it falls back to the legacy codepage, and
     * I would have to do conversion from UTF-16 to UTF-8 to avoid breaking on
     * Non-ASCII filenames.
     */
    uint32_t active_codepage = GetACP();
    // Like setlocale(LC_ALL, ""), but also ensures UTF-8 locale for Windows on UCRT.
    setlocale(LC_ALL, ".UTF-8");
    struct winver winver = get_win_version();

    if (active_codepage == 65001) {
        // UTF-8 mode (which is CP65001 in Windows)
        return 0;
    } else {
        printf("Windows version: %i.%i build %i\n", winver.major, winver.minor, winver.build);
        // Would be CP1252 if it fails in en-US locale (so a return value of 1252 in that case)
        return active_codepage;
    }
}

// Obviously, just return 0 if running on Unix-like OS. Use ifdefs for that.
// Return -1 if unsupported, 0 if good or no-op, error value if something else.
int enable_vt_escapes(void) {
    // I log to stderr
    HANDLE out = GetStdHandle(STD_ERROR_HANDLE);
    if (out == INVALID_HANDLE_VALUE) {
        int32_t err = GetLastError();
        print_win_err("GetStdHandle", err);
        return err;
    }
    DWORD cons_mode = 0;
    if (!GetConsoleMode(out, &cons_mode)) {
        int32_t err = GetLastError();
        print_win_err("GetConsoleMode", err);
        return err;
    }
    // Enable escape sequence processing
    cons_mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING
    if (!SetConsoleMode(out, cons_mode)) {
        // ERROR_INVALID_PARAMETER if not supported
        int32_t err = GetLastError();
        if (err == ERROR_INVALID_PARAMETER)
            return -1; // Indicate unsupported
        // Only runs if it is an error other than "unsupported"
        print_win_err("SetConsoleMode", err);
        return err;
    }
    return 0;
}

// Fix stdout to be binary so it doesn't corrupt binary files
void fix_stdout(void) {
    _setmode(_fileno(stdout), _O_BINARY);
}

#else
// We should be on a Unix-like system here, so these can be no-ops.
int ensure_locale(void) {
    // UTF-8 locale is assumed on Unix-like systems
    setlocale(LC_ALL, "");
    return 0;
}

int enable_vt_escapes(void) {
    return 0;
}
void fix_stdout(void) {
    return;
}
#endif // defined(_WIN32)
