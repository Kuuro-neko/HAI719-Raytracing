#include <random>
#include "Vec3.h"

#ifndef FUNCTIONS_H
#define FUNCTIONS_H

float random_float();
float random_float(float min, float max);
Vec3 random_unit_vector();
Vec3 random_on_hemisphere(const Vec3 &normal);
float min(float a, float b);
float max(float a, float b);
float clamp(float x, float min, float max);
Vec3 reflect(const Vec3 &direction_in, const Vec3 &n);
Vec3 refract(const Vec3 &direction_in, const Vec3 &n, float etai_over_etat);
float reflectance(float cosine, float ref_idx);
void gamma_correct(Vec3 &color);

#endif // FUNCTIONS_H
