// This contains per-OS functions to handle quirks.
#include <rt/quirks.h>

// OS-agnostic includes
// For setlocale()
#include <locale.h>

// std::clog
#include <iostream>

// OS-agnostic definitions
// Line printer function for usage in other functions
void (*rt::line_printer)(int);
// Prints "Done."
void (*rt::done_printer)();

// Print lines remaining (ANSI-escape version, hopefully faster on Windows)
void rt::print_lines_remaining_ansi(int lines_remaining) {
    // "\e[1G" moves to start of line, "\e[2K" clears from cursor to end of line.
    std::clog << "\033[22G\033[0K" << lines_remaining << std::flush;
}

// Print lines remaining (non-ANSI-terminal version)
// This runs *really* slowly on Windows compared to gnome-terminal.
void rt::print_lines_remaining_plain(int lines_remaining) {
    std::clog << "\rScanlines remaining: "
              << lines_remaining << ' ' << std::flush;
}

// Print "Done." (ANSI-escape version)
void rt::print_done_ansi(void) {
    std::clog << "\r\033[0KDone.\n";
}

// Print "Done." (non-ANSI fallback)
void rt::print_done_plain(void) {
    // Fallback when ANSI escapes are unavailable.
    std::clog << "\rDone.                 \n";
}

// Begin OS-specific definitions

#ifdef _WIN32
// FormatMessage(), LocalFree(), GetStdHandle(), GetConsoleMode(), SetConsoleMode(), GetLastError()
#include <windows.h>
// int32_t
#include <cstdint>
// printf(), wprintf()
#include <cstdio>
// These 2 are used for _setmode and _fileno, not sure which has which.
#include <fcntl.h>
#include <io.h>

// Internal variables
// Widechar args, which holds arguments in UTF-16. Can be freed by LocalFree()
wchar_t **wcargs = nullptr;
int wcargl = -1; // Holds length of wcargs (UTF-16 arguments)
// Holds cached reconverted args to save memory
char **conv_args = nullptr;
int conv_argl = -1;

using namespace rt;

// Utility functions

// This converts an integer error to a string error and logs it.
static void print_win_err(const char *func_name, uint32_t err) {
    LPTSTR msg_buf, display_buf;
    // FormatMessage(DWORD flags, LPCVOID src, DWORD msg_id, DWORD lang_id,
    //               LPTSTR buf (out), DWORD size, ...)
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                  FORMAT_MESSAGE_FROM_SYSTEM |
                  FORMAT_MESSAGE_IGNORE_INSERTS,
                  nullptr,
                  err,
                  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // User's language
                  (LPTSTR) &msg_buf, 0, nullptr);
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

// Initialize UTF-16 argument array for reconversion of arguments
static void init_wcargs(void) {
    // Get widechar (UTF-16) args in a manner similar to regular args
    wcargs = CommandLineToArgvW(GetCommandLineW(), &wcargl);
    if (wcargs == nullptr)
        std::clog << "CommandLineToArgvW failed!\n";
    else {
        conv_argl = wcargl;
        // Ensures pointers go to NULL
        conv_args = new char *[conv_argl]{nullptr};
    }
}


// Exported functions

// Returns 0 if fully successful (UTF-8 codepage), real codepage otherwise
int rt::ensure_locale(void) {
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
        std::clog << "Windows version: " << winver.major << '.' << winver.minor
                  << " build " << winver.build << '\n';
        // Would be CP1252 if it fails in en-US locale (so a return value of 1252 in that case)
        return active_codepage;
    }
}

// Obviously, just return 0 if running on Unix-like OS. Use ifdefs for that.
// Return -1 if unsupported, 0 if good or no-op, error value if something else.
int rt::enable_vt_escapes(void) {
    // I log to stderr
    HANDLE out = GetStdHandle(STD_ERROR_HANDLE);
    if (out == INVALID_HANDLE_VALUE) {
        uint32_t err = GetLastError();
        print_win_err("GetStdHandle", err);
        return err;
    }
    DWORD cons_mode = 0;
    if (!GetConsoleMode(out, &cons_mode)) {
        uint32_t err = GetLastError();
        print_win_err("GetConsoleMode", err);
        return err;
    }
    // Escape sequences already enabled.
    if (cons_mode & ENABLE_VIRTUAL_TERMINAL_PROCESSING)
        return 0;
    // Enable escape sequence processing
    cons_mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    if (!SetConsoleMode(out, cons_mode)) {
        // ERROR_INVALID_PARAMETER if not supported
        uint32_t err = GetLastError();
        if (err == ERROR_INVALID_PARAMETER)
            return -1; // Indicate unsupported
        // Only runs if it is an error other than "unsupported"
        print_win_err("SetConsoleMode", err);
        return err;
    }
    return 0;
}

// Fix stdout to be binary so it doesn't corrupt binary files
void rt::fix_stdout(void) {
    _setmode(_fileno(stdout), _O_BINARY);
}

// Retrieve reconverted CLI argument (which should be Unicode-clean)
// May return NULL (eventually crashing program) if conversion fails.
char * rt::reconv_cli_arg(int arg_pos, int argl, char *arg) {
    char *retarg;
    if (wcargs == nullptr) {
        init_wcargs();
        if (wcargs == nullptr) {
            std::clog << "Initalization of wcargs failed, bailing out.\n";
            return nullptr;
        }
    }

    if (argl != wcargl) {
        std::clog << "Hmm, that's weird. argl = " << argl << ", wcargl = "
                  << wcargl << ". All bets are off.\n";
    }
    /* int WideCharToMultiByte(unsigned int codepage, uint32_t flags,
     *                         wchar_t **instr, int inlen, char **outstr,
     *                         int outsize, char *default_char,
     *                         winbool default_char_used)
     * Note that winbool is actually an int internally. (Why?)
     * For more info, see:
     *  - https://learn.microsoft.com/en-us/windows/win32/api/stringapiset/nf-stringapiset-widechartomultibyte
     *  - https://learn.microsoft.com/en-us/windows/win32/winprog/windows-data-types
     */
    // Check if in array, so we don't allocate it multiple times
    if (conv_args[arg_pos] != nullptr) {
        return conv_args[arg_pos]; // Already converted
    }

    // I need to determine allocation size.
    int alloc_sz = WideCharToMultiByte(CP_UTF8, 0, wcargs[arg_pos],
                                       -1, // null-terminated
                                       nullptr, // unused
                                       0, // return size
                                       nullptr, // invalid otherwise
                                       nullptr); // Ditto
    if (alloc_sz == 0) {
        uint32_t err = GetLastError();
        print_win_err("WideCharToMultiByte (size check)", err);
        // This means it crashes when I try to use the argument.
        // For example, when attempting to open the file.
        return nullptr;
    }
    retarg = new char[alloc_sz];
    // Actually do conversion with appropriately sized buffer
    int status = WideCharToMultiByte(CP_UTF8, 0, wcargs[arg_pos], -1, retarg,
                                     alloc_sz, nullptr, nullptr);
    if (status == 0) {
        uint32_t err = GetLastError();
        print_win_err("WideCharToMultiByte", err);
        return nullptr;
    }
    // Store it for later use.
    conv_args[arg_pos] = retarg;
    return retarg;
}

#else
// We should be on a Unix-like system here, so these can be no-ops.
int rt::ensure_locale(void) {
    // UTF-8 locale is assumed on Unix-like systems
    setlocale(LC_ALL, "");
    return 0;
}

int rt::enable_vt_escapes(void) {
    return 0;
}
void rt::fix_stdout(void) {
    return;
}
char * rt::reconv_cli_arg([[maybe_unused]] int arg_pos,
                          [[maybe_unused]] int argl, char *fname) {
    return fname; // No conversion needed
}
#endif // defined(_WIN32)
