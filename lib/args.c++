// For std::clog and std::cout
#include <iostream>
// For exit()
#include <cstdlib>
// For std::string_view (requires C++17)
#include <string_view>
// For std::thread::hardware_concurrency()
#include <thread>
// For std::from_chars()
#include <charconv>

// For BMPOUT_*
#include <rt/bitmap.h>

// For struct args
#include <rt/args.h>

// Prints help
static void print_help(bool is_err, char *progname) {
    bool png_supported = bitmap::type_is_supported(BMPOUT_PNG);
    bool jpeg_supported = bitmap::type_is_supported(BMPOUT_JPEG);
    char help_str[] =
"\npositional arguments:\n"
"  FILE                  Specifies file to output to (defaults to stdout if\n"
"                        unspecified). File type is determined by extension if\n"
"                        not specified.\n"
"\noptional arguments:\n"
"  -h, --help            show this help message and exit\n"
"  -T NUM, --threads NUM Set the number of threads to run with. 0 is all threads\n"
"                        (the default setting).\n"
"  -t TYPE, --type TYPE  Set output file type. If stdout is specified, default\n"
"                        to ppm format. (Options: bmp, ppm";

    std::ostream &output = is_err? std::clog : std::cout;

    output << "usage: " << progname << " [-h] [-T NUM] [-t TYPE] [FILE]\n" << help_str;
    if (png_supported)
        output << ", png";
    if (jpeg_supported)
        output << ", jpg";
    output << ")\n";
}

// This class adds a few methods to string_view for convenience. It is
// otherwise the same. It adds case-insensitive ends_with() and fallbacks to
// enable C++17 support to be optionally used.

// To-Do: Make clone of operator ""sv?
// Note: virtual doesn't work when using superclass constructors on MSVC
// That is because std::string_view constructor is constexpr.
class StringView: public std::string_view {
  public:
    // This means I get constructor from std::string_view.
    using std::string_view::string_view;
  /* To do a call to superclass constructor, it looks something like this:
   *     StringView(): std::string_view() {}
   *     StringView(const char *in): std::string_view(in) {}
   * This is good to note if I need to do initialization in sub-object.
   */

    // I need assignment operator
    using std::string_view::operator=;

#ifndef __cpp_lib_starts_ends_with
    // Fallback implementation of starts_with()/ends_with() for C++17.
    // May be needed for older compilers.
    constexpr bool starts_with(std::string_view sv) const noexcept {
        if (length() < sv.length())
            return false;
        // Like substr(0, sv.length()).compare(sv)
        // string_view::compare() is like strcmp()
        if (compare(0, sv.length(), sv) == 0)
            return true;
        else
            return false;
    }
    constexpr bool starts_with(const char *str) const noexcept {
        std::string_view sv(str);
        return starts_with(sv);
    }
    constexpr bool starts_with(char c) const noexcept {
        if (!empty())
            return front() == c;
        else
            return false;
    }
    constexpr bool ends_with(std::string_view sv) const noexcept {
        if (length() < sv.length())
            return false;
        // This is like self[-len(sv):], and result is like strcmp()
        if (compare(length() - sv.length(), length(), sv) == 0)
            return true;
        else
            return false;
    }
    constexpr bool ends_with(const char *str) const noexcept {
        std::string_view sv(str);
        return ends_with(sv);
    }
    constexpr bool ends_with(char c) const noexcept {
        if (!empty())
            return back() == c;
        else
            return false;
    }
#endif

    // Case-insensitive "ends with" checking, needed to match extensions without using a regex.
    constexpr bool iends_with(std::string_view matchstr) {
        int bigchar, matchchar;
        // Semantically like len(bigstr) in Python
        int matchsize = matchstr.length();
        // Matched string can't be larger than the main string
        if (length() < matchstr.length())
            return false;
        // This evaluates like Python index bigstr[-matchsize]
        int bigstr_offset = length() - matchstr.length();
        for (int index = 0; index < matchsize; index++) {
            // I need to check that bigchar is correct, so do a bounds check
            bigchar = tolower(at(index + bigstr_offset));
            // No bounds check necessary for matchstr.
            matchchar = tolower(matchstr[index]);
            if (bigchar != matchchar)
                return false;
        }
        return true;
    }
    constexpr bool iends_with(const char *str) {
        std::string_view sv(str);
        return iends_with(sv);
    }
    constexpr bool iends_with(char c) {
        if (!empty()) {
            auto bigchar = tolower(back());
            auto matchchar = tolower(c);
            return bigchar == matchchar;
        } else
            return false;
    }
};

struct rt::args rt::parse_args(int argl, char **args) {
    // Check number of threads. If unassessable, it returns 0, which is changed to 1.
    int nproc = std::thread::hardware_concurrency();
    if (nproc == 0)
        nproc = 1;
    // Default argument values
    struct args parsed_args = {.fname_pos = -1, .ftype = BMPOUT_PPM,
                               .fname = nullptr, .n_threads = nproc};

    if (argl == 1)
        return parsed_args;
    int ftype_pos = -1;
    // I could use a bool here to indicate "next argument is threads" but then
    // I have to manually unset it at the handler.
    int n_threads_pos = -1;
    // To avoid resetting explicit type with implicit type
    bool type_is_set_explicitly = false;
    // Stop processing positional arguments
    bool no_more_options = false;
    // Explicit request to write to stdout
    bool stdout_explicit = false;

    using namespace std::string_view_literals; // needed for ""sv

    /* Implementation detail: std::string_view.starts_with() is only in C++20!
     * It can easily be reimplemented for the sake of targeting C++17, or this
     * code can be rewritten to support C++11 by not using std::string_view.
     */
    for (int index = 1; index < argl; index++) {
        // This makes it easier to deal with.
        StringView sv(args[index]);
        StringView type_name;
        StringView thread_num_string;
        bool set_type = false;
        bool set_fname = false;
        bool set_thread_num = false;

        if (no_more_options) {
            // It has been declared that there are no more positional arguments.
            set_fname = true;
        } else if (sv == "--help"sv || sv == "-h"sv) {
            print_help(false, args[0]); // Print help
            exit(0);
        } else if (ftype_pos == index) {
            // Here we engage the "set type in future" marker
            set_type = true;
            type_name = sv;
        } else if (sv == "--type"sv || sv == "-t"sv) {
            ftype_pos = index + 1; // Deal with this in the next iteration
            type_is_set_explicitly = true;
            continue; // Do next iteration
        } else if (sv.starts_with("--type=")) {
            set_type = true;
            // Slice sv[7:]
            type_name = sv.substr(7);
            type_is_set_explicitly = true;
        } else if (sv.starts_with("-t")) {
            set_type = true;
            // Slice sv[2:]
            type_name = sv.substr(2);
            type_is_set_explicitly = true;
        } else if (n_threads_pos == index) {
            set_thread_num = true;
            thread_num_string = sv;
        } else if (sv == "--threads"sv || sv == "-T"sv) {
            n_threads_pos = index + 1;
            continue; // Do next iteration
        } else if (sv.starts_with("--threads=")) {
            set_thread_num = true;
            // Slice sv[10:]
            thread_num_string = sv.substr(10);
        } else if (sv.starts_with("-T")) {
            set_thread_num = true;
            // Slice sv[2:]
            thread_num_string = sv.substr(2);
        } else if (sv == "--"sv) {
            no_more_options = true;
        } else if (sv == "-"sv) {
            // FIXME: This is supposed to handle the - (write to stdout) argument
            if (parsed_args.fname_pos != -1 || stdout_explicit) {
                print_help(true, args[0]);
                std::clog << "You cannot specify multiple files.\n";
                exit(1);
            }
            stdout_explicit = true;
        } else if (sv.starts_with("-")) {
            // Unrecognized option
            print_help(true, args[0]);
            std::clog << "Invalid option: " << sv << '\n';
            exit(1);
        } else {
            set_fname = true;
        }

        if (set_fname) {
            // Basic sanity check: make sure it isn't specified multiple times
            // Also, explicit stdout should never reach this code.
            if (parsed_args.fname_pos != -1 || stdout_explicit) {
                print_help(true, args[0]);
                std::clog << "You cannot specify multiple files.\n";
                exit(1);
            }
            // Set file name
            parsed_args.fname = args[index];
            parsed_args.fname_pos = index;

            if (!type_is_set_explicitly) {
                if (sv.iends_with(".bmp"sv))
                    parsed_args.ftype = BMPOUT_BMP;
                else if (sv.iends_with(".ppm"sv))
                    parsed_args.ftype = BMPOUT_PPM;
                else if (sv.iends_with(".png"sv)) {
                    parsed_args.ftype = BMPOUT_PNG;
                    if (!bitmap::type_is_supported(BMPOUT_PNG)) {
                        print_help(true, args[0]);
                        std::clog << "PNG support not built in.\n";
                        exit(4);
                    }
                } else if (sv.iends_with(".jpg"sv)) {
                    parsed_args.ftype = BMPOUT_JPEG;
                    if (!bitmap::type_is_supported(BMPOUT_JPEG)) {
                        print_help(true, args[0]);
                        std::clog << "JPEG support not built in.\n";
                        exit(4);
                    }
                } else {
                    print_help(true, args[0]);
                    std::clog << "Unrecognized extension on file " << sv << '\n';
                    exit(1);
                }
            }
        }

        if (set_type) {
            // Set (explicit) file type
            std::clog << "Setting file type explicitly\n";
            if (type_name == "bmp"sv)
                parsed_args.ftype = BMPOUT_BMP;
            else if (type_name == "ppm"sv)
                parsed_args.ftype = BMPOUT_PPM;
            else if (type_name == "png"sv) {
                parsed_args.ftype = BMPOUT_PNG;
                if (!bitmap::type_is_supported(BMPOUT_PNG)) {
                    print_help(true, args[0]);
                    std::clog << "PNG support not built in.\n";
                    exit(4);
                }
            } else if (type_name == "jpg"sv) {
                parsed_args.ftype = BMPOUT_JPEG;
                if (!bitmap::type_is_supported(BMPOUT_JPEG)) {
                    print_help(true, args[0]);
                    std::clog << "JPEG support not built in.\n";
                    exit(4);
                }
            } else {
                print_help(true, args[0]);
                std::clog << "Unrecognized file type: " << type_name << '\n';
                exit(1);
            }
        }

        if (set_thread_num) {
            // Set (explicit) number of threads
            auto [ptr, err] = std::from_chars(thread_num_string.data(),
                                              thread_num_string.data() + thread_num_string.size(),
                                              parsed_args.n_threads);

            if (err == std::errc::invalid_argument) {
                std::clog << "Not a number: " << thread_num_string << '\n';
                exit(1);
            } else if (err == std::errc::result_out_of_range) {
                std::clog << "Number is too large: " << thread_num_string << '\n';
                exit(1);
            }
            if (parsed_args.n_threads == 0)
                parsed_args.n_threads = nproc;
            else if (parsed_args.n_threads < 0) {
                print_help(true, args[0]);
                std::clog << "Number of threads must be 0 or greater (specified: "
                          << parsed_args.n_threads << ")\n";
                exit(1);
            }
        }
    }
    return parsed_args;
}

#ifdef ARG_TEST
// This exists to test the argument parser.
int main(int argl, char **args) {
    struct args pargs = parse_args(argl, args);

    const char *fname = "stdout";
    if (pargs.fname != nullptr)
        fname = pargs.fname;

    const char *ftype = "???";
    if (pargs.ftype == BMPOUT_PPM)
        ftype = "ppm";
    else if (pargs.ftype == BMPOUT_BMP)
        ftype = "bmp";

    std::cout << "fname is " << fname <<
                 "\nfname_pos is " << pargs.fname_pos <<
                 "\nftype is " << ftype << '\n';
}
#endif
