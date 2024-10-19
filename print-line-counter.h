#pragma once
// For std::clog
#include <iostream>

// Initial "lines remaining" line to allow more efficient counter.
static inline void print_first_lines_remaining(int lines_remaining) {
    std::clog << "Scanlines remaining: " << lines_remaining << std::flush;
}

#ifdef INCLUDE_REAL_PRINTER_FUNCS
// Define the actual symbols.

// Line printer function for usage in other functions
void (*line_printer)(int);
// Prints "Done."
void (*done_printer)();

// Print lines remaining (ANSI-escape version, hopefully faster on Windows)
static void print_lines_remaining_ansi(int lines_remaining) {
    // "\e[1G" moves to start of line, "\e[2K" clears from cursor to end of line.
    std::clog << "\033[22G\033[0K" << lines_remaining << std::flush;
}

// Print lines remaining (non-ANSI-terminal version)
// This runs *really* slowly on Windows compared to gnome-terminal.
static void print_lines_remaining_plain(int lines_remaining) {
    std::clog << "\rScanlines remaining: "
              << lines_remaining << ' ' << std::flush;
}

// Print "Done." (ANSI-escape version)
static void print_done_ansi(void) {
    std::clog << "\r\033[0KDone.\n";
}

// Print "Done." (non-ANSI fallback)
static void print_done_plain(void) {
    // Fallback when ANSI escapes are unavailable.
    std::clog << "\rDone.                 \n";
}
#else
// This specifies another file holds the actual pointer symbol.
// This allows the main program to define it and the library to hold it.
// Line printer function for usage in other functions
extern void (*line_printer)(int);
// Prints "Done."
extern void (*done_printer)();
#endif
