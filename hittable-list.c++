#include "hittable-list.h"

using std::make_shared;
using std::shared_ptr;

hittable_list::hittable_list(shared_ptr<hittable> object) {
    add(object);
}


void hittable_list::clear() {
    objects.clear();
}

void hittable_list::add(shared_ptr<hittable> object) {
    objects.push_back(object);
}


bool hit(const ray &r, double ray_tmin, double ray_tmax, hit_record &rec) const {
    hit_record temp_rec;
    bool hit_anything = false;
    auto closest_so_far = ray_tmax;

    // Is this a "for x in y" loop?
    for (const auto &object : objects) {
        if (object->hit(r, ray_tmin, closest_so_far, temp_rec)) {
            hit_anything = true;
            closest_so_far = temp_rec.t;
            rec = temp_rec;
        }
    }

    return hit_anything;
}
