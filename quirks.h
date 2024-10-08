#pragma once
// This contains functions to work around OS quirks like Windows not having
// ANSI escapes by default, or "\n" -> "\r\n" replacement on stdout.

#ifdef __cplusplus
extern "C" {
#endif

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

#ifdef __cplusplus
}
#endif
