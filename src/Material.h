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

enum TextureType {
    Texture_None,
    Texture_Checkerboard,
    Texture_Image
};

struct Material {
    Vec3 ambient_material;
    Vec3 diffuse_material;
    Vec3 specular_material;
    double shininess;

    float index_medium;
    float transparency;

    MaterialType type;
    TextureType texture_type;
    
    Vec3 checkerboard_color1;
    Vec3 checkerboard_color2;
    float checkerboard_scale;

    ppmLoader::ImageRGB image;

    Material();

    void scatter(const Ray &ray_in, const Vec3 &normal, const Vec3 &intersection, Ray &ray_out);

    void texture(Vec3 &color, float u, float v);
    void sphere_texture(Vec3 &color, const float phi, const float theta);
    void load_texture(const std::string &filename);
};

#endif // MATERIAL_H
