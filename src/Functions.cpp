#include "Functions.h"

float random_float() {
    static std::uniform_real_distribution<float> distribution(0.0, 1.0);
    static std::mt19937 generator(static_cast<unsigned int>(time(nullptr)));
    return distribution(generator);
}

float random_float(float min, float max) {
    return min + (max - min) * random_float();
}


/*
float random_float() {
    return (float)rand() / RAND_MAX;
}

float random_float(float min, float max) {
    return min + (max - min) * random_float();
}*/

Vec3 random_unit_vector() {
    Vec3 p = Vec3(random_float(-1, 1), random_float(-1, 1), random_float(-1, 1));
    p.normalize();
    return p;
}

Vec3 random_on_hemisphere(const Vec3 &normal) {
    Vec3 random_vector = random_unit_vector();
    if (Vec3::dot(random_vector, normal) > 0.0) {
        return random_vector;
    } else {
        return random_vector*-1.;
    }
}

float min(float a, float b) {
    return a < b ? a : b;
}

float max(float a, float b) {
    return a > b ? a : b;
}

float clamp(float x, float min, float max) {
    if (x < min) {
        return min;
    }
    if (x > max) {
        return max;
    }
    return x;
}

Vec3 reflect(const Vec3 &direction_in, const Vec3 &n) {
    return direction_in - 2 * Vec3::dot(direction_in, n) * n;
}

Vec3 refract(const Vec3 &direction_in, const Vec3 &n, float etai_over_etat) {
    float cos_theta = min(Vec3::dot(direction_in, n), 1.0);
    Vec3 r_out_perp = etai_over_etat * (direction_in + cos_theta * n);
    Vec3 r_out_parallel = -sqrt(fabs(1.0 - r_out_perp.squareLength())) * n;
    return r_out_perp + r_out_parallel;
}

float reflectance(float cosine, float ref_idx) {
    // Use Schlick's approximation for reflectance.
    float r0 = (1 - ref_idx) / (1 + ref_idx);
    r0 = r0 * r0;
    return r0 + (1 - r0) * pow((1 - cosine), 5);
}

void gamma_correct(Vec3 &color) {
    color[0] = pow(color[0], 1.0/2.2);
    color[1] = pow(color[1], 1.0/2.2);
    color[2] = pow(color[2], 1.0/2.2);
}