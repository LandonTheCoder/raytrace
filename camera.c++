#include "camera.h"
#include <iostream>
// For bitmap class
#include "bitmap.h"
// For random_double()
#include "utils.h"
// For material to scatter
#include "material.h"

// For OS-specific workarounds/quirks
#include "quirks.h"

// For std::thread
#include <thread>
// For std::vector
#include <vector>

// Internal implementation of a renderer thread.
void camera::render_mt_impl(const hittable &world, bitmap &raw_bmp, int line_begin, int line_end) {
    // Assume it is already initialized, and that lines_remaining is correct.

    for (int j = line_begin; j < line_end; j++) {
        // I assume the first status line has been printed.
        for (int i = 0; i < image_width; i++) {
            color pixel_color(0, 0, 0);
            for (int sample = 0; sample < samples_per_pixel; sample++) {
                ray r = get_ray(i, j);
                pixel_color += ray_color(r, max_depth, world);
            }

            raw_bmp.write_pixel_vec3(j, i, pixel_samples_scale * pixel_color);
        }
        // I have to lock access to lines_remaining and clog.
        // Note: This causes some slight overhead with enough contention.
        {
            std::lock_guard<std::mutex> print_guard(counter_mutex);
            lines_remaining--;
            std::clog << "\rScanlines remaining: " << lines_remaining << ' ' << std::flush;
        }
    }
}

// Multithreaded renderer function. It launches a set of renderer threads.
bitmap camera::render(const hittable &world, int n_threads) {
    if (n_threads < 0)
        throw std::invalid_argument("The number of threads must be 0 or greater!");

    int vt_escape_status = enable_vt_escapes();

    // Note: render_mutex is a recursive mutex because it will also be locked from
    // the single-threaded renderer if we decide to call it.
    std::unique_lock<std::recursive_mutex> render_lock(render_mutex, std::try_to_lock);

    if (!render_lock) {
        // The "WARNING" text shows in bold red if ANSI escapes are available
        std::clog << (vt_escape_status==0? "\033[1;31mWARNING\033[0m" : "WARNING")
                  << ": Trying to render when already rendering! "
                     "Thread will hang until previous job finishes.\n";
        // Hangs until existing job is complete.
        render_lock.lock();
    }

    if (n_threads == 0) {
        int nproc = std::thread::hardware_concurrency();
        if (nproc == 0)
            nproc = 1;
        std::clog << "Setting number of threads automatically to " << nproc << '\n';
        n_threads = nproc;
    }
    if (n_threads == 1) {
        std::clog << "Using single-threaded implementation.\n";
        // This is done before camera::initialize() to avoid any change in behavior.
        auto raw_bmp = render(world);
        return raw_bmp;
    }

    initialize();

    // Placed here because image_height is set in camera::initialize()
    if (n_threads > image_height) {
        // Implementation detail: I can't have more threads than image lines
        std::clog << "More threads requested than possible: reducing " << n_threads
                  << " to " << image_height << '\n';
        n_threads = image_height;
    }

    auto raw_bmp = bitmap(image_width, image_height);

    std::clog << "Using " << n_threads << " threads.\n";

    auto block_size = image_height / n_threads;
    auto block_remainder = image_height % n_threads; // Assigned to the last thread

    std::vector<std::thread> thread_list;

    auto begin_idx = 0;
    auto end_idx = block_size;

    lines_remaining = image_height;
    std::clog << "\rScanlines remaining: " << lines_remaining << std::flush;

    for (int tid = 0; tid < n_threads; tid++) {
        // The remainder is given to the last thread.
        if (tid == n_threads - 1)
            end_idx += block_remainder;

        // thread_list runs the constructor itself.
        thread_list.push_back(std::thread(&camera::render_mt_impl, this,
                                          std::cref(world),
                                          std::ref(raw_bmp),
                                          begin_idx, end_idx));

        begin_idx = end_idx;
        end_idx += block_size;
    }

    for (auto &t: thread_list) {
        t.join();
    }

    // I wonder if this is breaking things somehow. Try commenting it out?
    lines_remaining = -1;

    if (vt_escape_status == 0) {
        // Looks nicer. This clears cursor to end of line.
        std::clog << "\r\033[0KDone.\n";
    } else {
        // Fallback output if ANSI escapes are unavailable.
        std::clog << "\rDone.                 \n";
    }

    return raw_bmp;
}

bitmap camera::render(const hittable &world) {
    // Returns 0 if success/no-op, -1 if unavailable, positive error otherwise
    int vt_escape_status = enable_vt_escapes();

    // I have to make sure it isn't trying to run 2 jobs at once (data race!)
    // This may be locked twice within the same thread if called from the MT renderer method.
    std::unique_lock<std::recursive_mutex> render_lock(render_mutex, std::try_to_lock);

    if (!render_lock) {
        // The "WARNING" text shows in bold red if ANSI escapes are available
        std::clog << (vt_escape_status==0? "\033[1;31mWARNING\033[0m" : "WARNING")
                  << ": Trying to render when already rendering! "
                     "Thread will hang until previous job finishes.\n";
        // Hangs until existing job is complete.
        render_lock.lock();
    }

    initialize();


    // Fill in with code from main()
    auto raw_bmp = bitmap(image_width, image_height);

    // Go through image from left-to-right, top-to-bottom.
    for (int j = 0; j < image_height; j++) {
        std::clog << "\rScanlines remaining: " << (image_height - j) << ' ' << std::flush;
        for (int i = 0; i < image_width; i++) {
            color pixel_color(0, 0, 0);
            for (int sample = 0; sample < samples_per_pixel; sample++) {
                ray r = get_ray(i, j);
                pixel_color += ray_color(r, max_depth, world);
            }

            raw_bmp.write_pixel_vec3(j, i, pixel_samples_scale * pixel_color);
        }
    }

    if (vt_escape_status == 0) {
        // Looks nicer. This clears cursor to end of line.
        std::clog << "\r\033[0KDone.\n";
    } else {
        // Fallback output if ANSI escapes are unavailable.
        std::clog << "\rDone.                 \n";
    }

    return raw_bmp;
}

// Initialize variables
void camera::initialize() {
    image_height = int(image_width/aspect_ratio);
    image_height = (image_height < 1)? 1: image_height;

    // We are averaging out the pixel color based on n samples.
    pixel_samples_scale = 1.0 / samples_per_pixel;

    center = lookfrom;

    // Determine dimensions of viewport.
    auto theta = degrees_to_radians(vfov);
    auto h = std::tan(theta/2);
    // Viewport is the region through which the scene rays pass.
    auto viewport_height = 2.0 * h * focus_dist;
    auto viewport_width = viewport_height * (double(image_width)/image_height);

    // Find frame-basis vectors for camera-coordinate frame
    w = unit_vector(lookfrom - lookat);
    u = unit_vector(cross(vup, w));
    v = cross(w, u);

    // Calculate the vectors across the horizontal and down the vertical viewport edges.
    // Viewport_u is left-to-right, viewport_v is top-to-bottom.
    vec3 viewport_u = viewport_width * u; // Vector across horiz. edge
    vec3 viewport_v = viewport_height * -v; // Vector down vert. edge (y-axis is inverted)

    // Calculate the horizontal and vertical delta vectors from pixel to pixel.
    pixel_delta_u = viewport_u / image_width;
    pixel_delta_v = viewport_v / image_height;

    // Calculate the location of the upper left pixel.
    auto viewport_upper_left = center - (focus_dist * w) - viewport_u/2 - viewport_v/2;
    pixel00_loc = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);

    // Calculate camera defocus disk basis vectors
    auto defocus_radius = focus_dist * std::tan(degrees_to_radians(defocus_angle / 2));
    defocus_disk_u = u * defocus_radius;
    defocus_disk_v = v * defocus_radius;
}

ray camera::get_ray(int i, int j) {
    /* We build a camera ray which originates from defocus disk and is directed at a
     * randomly sampled point near pixel (i, j)
     */
    auto offset = sample_square();

    auto pixel_sample = (pixel00_loc
                         + ((i + offset.x()) * pixel_delta_u)
                         + ((j + offset.y()) * pixel_delta_v));

    auto ray_origin = defocus_angle <= 0? center: defocus_disk_sample();
    auto ray_direction = pixel_sample - ray_origin;

    return ray(ray_origin, ray_direction);
}

vec3 camera::sample_square() const {
    // Returns vector to a random point in the square encompassing ([-.5, .5], [-.5, .5])
    return vec3(random_double() - 0.5, random_double() - 0.5, 0);
}

// At a = 0 it is white, at a = 1.0 it is blue, blend in between.
color camera::ray_color(const ray &r, int depth, const hittable &world) {
    // Don't gather any more light if max depth is exceeded
    if (depth <= 0)
        return color(0, 0, 0);

    hit_record rec;

    // Ignore really close hits to hack around "shadow acne" problem
    if (world.hit(r, interval(0.001, infinity), rec)) {
        ray scattered;
        color attenuation;
        // Recurse until it stops hitting something or exceeds max depth.
        if (rec.mat->scatter(r, rec, attenuation, scattered))
            return attenuation * ray_color(scattered, depth - 1, world);
        return color(0, 0, 0);
    }

    vec3 unit_direction = unit_vector(r.direction());
    auto a = 0.5 * (unit_direction.y() + 1.0);
    // This is a linear interpolation.
    return (1.0 - a) * color(1.0, 1.0, 1.0) + a * color(0.5, 0.7, 1.0);
}

point3 camera::defocus_disk_sample() const {
    // Return random point in camera defocus disk
    auto p = random_in_unit_disk();
    return center + (p[0] * defocus_disk_u) + (p[1] * defocus_disk_v);
}
