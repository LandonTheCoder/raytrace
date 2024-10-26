#pragma once
// This contains functions to work around OS quirks like Windows not having
// ANSI escapes by default, or "\n" -> "\r\n" replacement on stdout.

// For std::clog
#include <iostream>

namespace rt {

// This specifies another file holds the actual pointer symbol.
// This allows the main program to define it and the library to hold it.
// Line printer function for usage in other functions
extern void (*line_printer)(int);
// Prints "Done."
extern void (*done_printer)();

static inline void print_first_lines_remaining(int lines_remaining) {
    std::clog << "Scanlines remaining: " << lines_remaining << std::flush;
}

void print_lines_remaining_ansi(int lines_remaining);
void print_lines_remaining_plain(int lines_remaining);
void print_done_ansi(void);
void print_done_plain(void);

/* Sets locale correctly. This ensures a locale other than the "C" locale,
 * and on Windows ensures UTF-8 locale if possible.
 * Returns 0 if no-op or UTF-8 ensured. On Windows, returns active codepage
 * if it fails to have UTF-8 ACP (thus needing filename reconversion).
 * Note that filename conversion is not currently implemented, so it will
 * fail to open non-ASCII filenames.
 */
int ensure_locale(void);

/* Enables ANSI escapes if available.
 * Returns 0 if successful (or no-op on Unix-like systems),
 * -1 if unsupported, a positive error code otherwise.
 */
int enable_vt_escapes(void);

/* On Windows, sets stdout (and hopefully std::cout?) to binary mode to avoid
 * undesired conversion of "\n" to "\r\n" (which would corupt binary files).
 * No-op on Unix-like systems.
 */
void fix_stdout(void);

/* Reconverts the CLI argument if necessary. Otherwise, returns existing
 * argument again so it can be used more conveniently.
 * arg_pos is the index in the argument array, argl is a.k.a. argc,
 * arg is the existing argument.
 * Note: If it were reallocated, the caller would have to free(), but I
 * don't really see any reason to free() an argument anyways.
 */
char * reconv_cli_arg(int arg_pos, int argl, char *arg);

}
