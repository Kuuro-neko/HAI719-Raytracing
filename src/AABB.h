#ifndef AABB_H
#define AABB_H

#include "Vec3.h"
#include <vector>
#include "Functions.h"
#include "Constants.h"
#include "Ray.h"
#include <cfloat>

struct AABBCuttingPlane {
    float position;
    unsigned int axis;

    AABBCuttingPlane(unsigned int axis, float position) : axis(axis), position(position) {}
    AABBCuttingPlane() : axis(0), position(0) {}
};

class AABB {
    public:
        Vec3 p0;
        Vec3 p1;

    AABB() {
        p0 = Vec3(FLT_MAX);
        p1 = Vec3(-FLT_MAX);
    }

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

    void extend(const AABB &aabb) {
        for (int i = 0; i < 3; i++) {
            p0[i] = min(p0[i], aabb.p0[i]);
            p1[i] = max(p1[i], aabb.p1[i]);
        }
    }

    bool intersects(const Ray &ray, float tmin=EPSILON, float tmax=FLT_MAX) const {
        for (int axis = 0; axis < 3; axis++) {
            const double adinv = 1.0 / ray.direction()[axis];

            float t0 = (p0[axis] - ray.origin()[axis]) * adinv;
            float t1 = (p1[axis] - ray.origin()[axis]) * adinv;

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

    std::pair<AABB, AABB> split(const AABBCuttingPlane &plane) const {
        AABB left = *this;
        AABB right = *this;

        left.p1[plane.axis] = plane.position;
        right.p0[plane.axis] = plane.position;

        return std::make_pair(left, right);
    }
};

#endif // AABB_H