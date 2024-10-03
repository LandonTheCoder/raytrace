// For std::clog and std::cout
#include <iostream>
// For exit()
#include <cstdlib>
// For std::string_view (requires C++17)
#include <string_view>

// For BMPOUT_*
#include "bitmap.h"

// For struct args
#include "args.h"

// Prints help
static void print_help(bool is_err, char *progname) {
    char help_str[] =
"\npositional arguments:\n"
"  FILE                  Specifies file to output to (defaults to stdout if\n"
"                        unspecified). File type is determined by extension if\n"
"                        not specified.\n"
"\noptional arguments:\n"
"  -h, --help            show this help message and exit\n"
"  -t TYPE, --type TYPE  Set output file type (options: ppm, bmp). If stdout is\n"
"                        specified, default to ppm format\n";

    if (is_err)
        std::clog << "usage: " << progname << " [-h] [-t TYPE] [FILE]\n" << help_str;
    else
        std::cout << "usage: " << progname << " [-h] [-t TYPE] [FILE]\n" << help_str;
}

// Case-insensitive "ends with" checking, needed to match extensions without using a regex.
static bool iendswith(std::string_view bigstr, std::string_view matchstr) {
    int bigchar, matchchar;
    // Semantically like len(bigstr) in Python
    int bigsize = bigstr.length();
    int matchsize = matchstr.length();
    // Matched string can't be larger than the main string
    if (bigsize < matchsize)
        return false;
    // This evaluates like Python index bigstr[-matchsize]
    int bigstr_offset = bigsize - matchsize;
    for (int index = 0; index < matchsize; index++) {
        // I need to check that bigchar is correct, so do a bounds check
        bigchar = tolower(bigstr.at(index + bigstr_offset));
        // No bounds check necessary for matchstr.
        matchchar = tolower(matchstr[index]);
        if (bigchar != matchchar)
            return false;
    }
    return true;
}

struct args parse_args(int argl, char **args) {
    // Default argument values
    struct args parsed_args = {.fname_pos = -1, .ftype = BMPOUT_PPM, .fname = nullptr};

    if (argl == 1)
        return parsed_args;
    int ftype_pos = -1;
    // To avoid resetting explicit type with implicit type
    bool type_is_set_explicitly = false;
    // Stop processing positional arguments
    bool no_more_options = false;

    using namespace std::string_view_literals; // needed for ""sv

    /* Implementation detail: std::string_view.starts_with() is only in C++20!
     * It can easily be reimplemented for the sake of targeting C++17, or this
     * code can be rewritten to support C++11 by not using std::string_view.
     */
    for (int index = 1; index < argl; index++) {
        // This makes it easier to deal with.
        std::string_view sv(args[index]);
        std::string_view type_name;
        bool set_type = false;
        bool set_fname = false;

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
            // Slice sv[7:]
            type_name = sv.substr(7);
            type_is_set_explicitly = true;
        } else if (sv.starts_with("-t")) {
            // Slice sv[1:]
            type_name = sv.substr(1);
            type_is_set_explicitly = true;
        } else if (sv == "--"sv) {
            no_more_options = true;
        } else if (sv == "-"sv) {
            // FIXME: This is supposed to handle the - (write to stdout) argument
            ;
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
            if (parsed_args.fname_pos != -1) {
                print_help(true, args[0]);
                std::clog << "You cannot specify multiple files.\n";
                exit(1);
            }
            // Set file name
            parsed_args.fname = args[index];
            parsed_args.fname_pos = index;

            if (!type_is_set_explicitly) {
                if (iendswith(sv, ".bmp"sv))
                    parsed_args.ftype = BMPOUT_BMP;
                else if (iendswith(sv, ".ppm"sv))
                    parsed_args.ftype = BMPOUT_PPM;
                else {
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
            else {
                print_help(true, args[0]);
                std::clog << "Unrecognized file type: " << type_name << '\n';
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
