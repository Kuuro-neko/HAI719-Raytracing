#ifndef MATERIAL_H
#define MATERIAL_H

#include "imageLoader.h"
#include "Vec3.h"
#include <cmath>
#include "Ray.h"
#include "Functions.h"
#include <GL/glut.h>

enum MaterialType {
    Material_Diffuse_Blinn_Phong,
    Material_Glass,
    Material_Mirror
};

struct Material {
    Vec3 ambient_material;
    Vec3 diffuse_material;
    Vec3 specular_material;
    double shininess;

    float index_medium;
    float transparency;

    MaterialType type;

    Material();

    void scatter(const Ray &ray_in, const Vec3 &normal, const Vec3 &intersection, Ray &ray_out);
};

#endif // MATERIAL_H
