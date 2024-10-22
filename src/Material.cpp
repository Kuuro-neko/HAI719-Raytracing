#include "Material.h"
#include "imageLoader.h"    

Material::Material() {
    type = Material_Diffuse_Blinn_Phong;
    texture_type = Texture_None;
    transparency = 0.0;
    index_medium = 1.0;
    ambient_material = Vec3(0., 0., 0.);
}

void Material::scatter(const Ray &ray_in, const Vec3 &normal, const Vec3 &intersection, Ray &ray_out) {
    float ri, cos_theta, sin_theta;
    bool cannot_refract;
    Vec3 direction;
    switch (type) {
        case Material_Glass:
            if (Vec3::dot(ray_in.direction(), normal) > 0) {
                ri = 1./index_medium;
            } else {
                ri = index_medium;
            }/*
            cos_theta = min(Vec3::dot(ray_in.direction()*-1., normal), 1.0);
            sin_theta = sqrt(1. - cos_theta*cos_theta);
            cannot_refract = ri * sin_theta > 1.0;
            if (cannot_refract || reflectance(cos_theta, ri) > random_float()) {
                direction = reflect(ray_in.direction(), normal);
            } else {
                direction = refract(ray_in.direction(), normal, ri);
            }*/
            direction = refract(ray_in.direction(), normal, ri);
            break;
        case Material_Diffuse_Blinn_Phong:
            direction = normal + random_unit_vector();
            if (direction.length() <= EPSILON) {
                direction = normal;
            }
            break;
        case Material_Mirror:
            direction = reflect(ray_in.direction(), normal);
            break;
        default:
            break;
    }
    direction.normalize();
    ray_out = Ray(intersection + EPSILON * direction, direction);
}


void Material::texture(Vec3 &color, float u, float v) {
    int x, y, index;
    switch (texture_type) {
        case Texture_Checkerboard:
            if ((int)(u*checkerboard_scale) % 2 == (int)(v*checkerboard_scale) % 2) {
                color = checkerboard_color1;
            } else {
                color = checkerboard_color2;
            }
            break;
        case Texture_Image:
            if (image.w < 1 || image.h < 1) {
                    if ((int)(u*8.) % 2 == (int)(v*8.) % 2) {
                        color = Vec3(0., 0., 0.);
                    } else {
                        color = Vec3(1.,0.,1.);   
                    }
                break;
            }
            u = clamp(u, 0., 1.);
            v = 1. - clamp(v, 0., 1.);
            x = int(u * image.w);
            y = int(v * image.h);
            index = y * image.w + x;
            color = Vec3(image.data[index].r/255., image.data[index].g/255., image.data[index].b/255.);
            break;
        default:
            break;
    }
}

void Material::sphere_texture(Vec3 &color, const float phi, const float theta) {
    switch (texture_type) {
        case Texture_Checkerboard:
        case Texture_Image:
            texture(color, phi/(2*M_PI), theta/M_PI);
            break;
        default:
            break;
    }
}

void Material::load_texture(const std::string &filename) {
    ppmLoader::load_ppm(image, filename);
}
