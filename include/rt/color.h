#pragma once

#include "vec3.h"

namespace rt {
// Some things use a color type, but don't need the bitmap writer.
using color = vec3;
}
