#ifndef AABB_H
#define AABB_H

#include "Vec3.h"
#include <vector>
#include "Functions.h"
#include "Ray.h"
#include <cfloat>

class AABB {
    public:
        Vec3 p0;
        Vec3 p1;

    AABB() {}

    AABB(const Vec3 &a, const Vec3 &b) {
        for (int i = 0; i < 3; i++) {
            if (a[i] < b[i]) {
                p0[i] = a[i];
                p1[i] = b[i];
            } else {
                p1[i] = a[i];
                p0[i] = b[i];
            }
        }
    }

    bool intersects(const Ray &ray, float tmin=EPSILON, float tmax=FLT_MAX) const {
        for (int axis = 0; axis < 3; axis++) {
            const double adinv = 1.0 / ray.direction()[axis];

            auto t0 = (p0[axis] - ray.origin()[axis]) * adinv;
            auto t1 = (p1[axis] - ray.origin()[axis]) * adinv;

            if (t0 < t1) {
                if (t0 > tmin) tmin = t0;
                if (t1 < tmax) tmax = t1;
            } else {
                if (t1 > tmin) tmin = t1;
                if (t0 < tmax) tmax = t0;
            }
            if (tmax <= tmin) return false;
        }
        return true;
    }   
};

#endif // AABB_H