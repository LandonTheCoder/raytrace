#include <rt/color.h>
#include <rt/bitmap.h>
#include <rt/vec3.h>
#include <rt/interval.h>
// For struct args, parse_args()
#include <rt/args.h>
// For testing workarounds/quirks, line counter
#include <rt/quirks.h>

#include <iostream>
// For std::ofstream
#include <fstream>
// For signal handling
#include <csignal>
// For std::filesystem::path (used in signal handler)
#include <filesystem>

using namespace rt;

// Converts to RGB in linear space (unlike the bitmap method)
struct rgb color_vec3_to_rgb(const color &pixel_color) {
    auto r = pixel_color.x();
    auto g = pixel_color.y();
    auto b = pixel_color.z();

    // We now do a clamped translation from [0.0, 1.0] to [0, 255]
    static const interval intensity(0.000, 0.999);

    uint8_t rbyte = uint8_t(256 * intensity.clamp(r));
    uint8_t gbyte = uint8_t(256 * intensity.clamp(g));
    uint8_t bbyte = uint8_t(256 * intensity.clamp(b));

    // Return components
    return {.r = rbyte, .g = gbyte, .b = bbyte};
}

static std::filesystem::path fpath;
static std::ofstream out_file;

// This is like ppm-example but not hardcoded for ppm to stdout.
int main(int argl, char **args) {
    // Testing quirks.
    int locale_is_good = ensure_locale();
    // Returns 0 if success/no-op, -1 if unavailable, positive error code otherwise.
    int vt_escape_status = enable_vt_escapes();
    // Select line printer function
    line_printer = vt_escape_status==0?
                   print_lines_remaining_ansi :
                   print_lines_remaining_plain;
    // Select done printer function
    done_printer = vt_escape_status==0? print_done_ansi: print_done_plain;
    fix_stdout();

    struct args pargs = parse_args(argl, args);

    // If not UTF-8, it returns the codepage in locale_is_good
    if (locale_is_good > 0) {
        // Terminal escapes supported, so do fancy print
        // "\e[1m" is bold, "\e[31m" is red, "\e[0m" is reset
        char acperr_ansi[] = "\033[1;31mWARNING\033[0m: Not running in UTF-8 mode! Non-ASCII "
                             "filenames may fail to open! Running in codepage ";
        // The boring version
        char acperr_plain[] = "WARNING: Not running in UTF-8 mode! Non-ASCII "
                              "filenames may fail to open! Running in codepage ";
        // Determine which version to print.
        std::clog << (vt_escape_status == 0? acperr_ansi : acperr_plain)
                  << locale_is_good << '\n';

        // Experimental: Try to reconvert filename
        char *temp = reconv_cli_arg(pargs.fname_pos, argl, pargs.fname);
        if (temp == nullptr) {
            std::clog << "Filename conversion failed.\n";
            return 6;
        }
        pargs.fname = temp;
    }

    if (pargs.fname != nullptr) {
        // Check if this works correctly on UTF-8 clean versions of Windows
        // Binary is needed to ensure it doesn't butcher 0x0A on Windows
        out_file.open(pargs.fname, std::ios_base::out
                                   | std::ios_base::binary
                                   | std::ios_base::trunc);
        fpath = std::filesystem::path(pargs.fname);
        // It doesn't let me use closures :(
        // I want to delete the empty file when I exit.
        std::signal(SIGINT, [](int signum) -> void {
            // Note: This is because it seems I *have* to close before deleting on Windows.
            out_file.close();
            std::filesystem::remove(fpath);
            if (signum == SIGINT)
                std::exit(0);
            else
                std::exit(128 + signum); // Unix default signal behavior
        });
    }

    // Image

    int image_width = 256;
    int image_height = 256;

    // Render

    auto raw_bmp = bitmap(image_width, image_height);

    print_first_lines_remaining(image_height);
    // PPM is written left-to-right, top-to-bottom.
    for (int j = 0; j < image_height; j++) {
        line_printer(image_height - j);
        for (int i = 0; i < image_width; i++) {
            auto pixel_color = color(double(i) / (image_width - 1), double(j) / (image_height - 1), 0.0);

            auto pixel_color_rgb = color_vec3_to_rgb(pixel_color);

            // The write_pixel_vec3 does a gamma interpolation, so I convert manually here.
            raw_bmp.write_pixel_rgb(j, i, pixel_color_rgb.r, pixel_color_rgb.g, pixel_color_rgb.b);
        }
    }
    // Handle arguments
    std::ostream &outstream = (pargs.fname != nullptr)? out_file : std::cout;

    if (pargs.ftype == BitmapOutput::PPM) {
        raw_bmp.write_as_ppm(outstream);
    } else if (pargs.ftype == BitmapOutput::BMP) {
        raw_bmp.write_as_bmp_btt(outstream);
    } else if (pargs.ftype == BitmapOutput::PNG) {
        raw_bmp.write_as_png(outstream);
    } else if (pargs.ftype == BitmapOutput::JPEG) {
        raw_bmp.write_as_jpeg(outstream);
    }

    done_printer();
}
