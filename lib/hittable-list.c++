#include <rt/hittable-list.h>

using std::make_shared;
using std::shared_ptr;
using rt::ray;
using rt::hittable;
using rt::interval;
using rt::hit_record;

rt::hittable_list::hittable_list(shared_ptr<hittable> object) {
    add(object);
}


void rt::hittable_list::clear() {
    objects.clear();
}

void rt::hittable_list::add(shared_ptr<hittable> object) {
    objects.push_back(object);
}


bool rt::hittable_list::hit(const ray &r, interval ray_t, hit_record &rec) const {
    hit_record temp_rec;
    bool hit_anything = false;
    auto closest_so_far = ray_t.max;

    // Is this a "for x in y" loop?
    for (const auto &object : objects) {
        if (object->hit(r, interval(ray_t.min, closest_so_far), temp_rec)) {
            hit_anything = true;
            closest_so_far = temp_rec.t;
            rec = temp_rec;
        }
    }

    return hit_anything;
}
