# Raytrace in a Weekend #
See URL [Ray Tracing in One Weekend Series](https://raytracing.github.io), and book [Ray Tracing in a Weelend](https://raytracing.github.io/books/RayTracingInOneWeekend.html).

The basic gist of what I'm doing is:
1. Calculate the ray from the “eye” through the pixel,
2. Determine which objects the ray intersects, and
3. Compute a color for the closest intersection point.

This is based on the Ray Tracing in One Weekend tutorial, with a few modifications added for flexibility (such as modifying it so that it can output to formats other than PPM, with room for expansion).
