#include "Material.h"
#include "imageLoader.h"    

Material::Material() {
    type = Material_Diffuse_Blinn_Phong;
    texture_type = Texture_None;
    transparency = 0.0;
    index_medium = 1.0;
    ambient_material = Vec3(0., 0., 0.);
}

void Material::emit(Vec3 &color, float u, float v) {
    if (!emissive) {
        color = Vec3(0., 0., 0.);
        return;
    }
    if (texture_type == Texture_None) {
        color = light_color;
    } else {
        texture(color, u, v);
    }
    color *= light_intensity;
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
            }
            cos_theta = min(Vec3::dot(ray_in.direction()*-1., normal), 1.0);
            sin_theta = sqrt(1. - cos_theta*cos_theta);
            cannot_refract = (ri * sin_theta)-0.6 > 1.0;
            if (cannot_refract || reflectance(cos_theta, ri) > random_float()) {
                direction = reflect(ray_in.direction(), normal);
            } else {
                direction = refract(ray_in.direction(), normal, ri);
            }
            //direction = refract(ray_in.direction(), normal, ri);
            break;
        case Material_Diffuse_Blinn_Phong:
            direction = normal + random_unit_vector();
            if (direction.length() <= EPSILON) {
                direction = normal;
            }
            break;
        case Material_Mirror:
            direction = reflect(ray_in.direction(), normal); //+ random_unit_vector() * 0.01; fuzziness
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
            if (image->w < 1 || image->h < 1) {
                    if ((int)(u*8.) % 2 == (int)(v*8.) % 2) {
                        color = Vec3(0., 0., 0.);
                    } else {
                        color = Vec3(1.,0.,1.);   
                    }
                break;
            }
            u = clamp(u, 0., 1.);
            v = 1. - clamp(v, 0., 1.);
            x = int(u * (image->w - 1));
            y = int(v * (image->h - 1));
            index = y * image->w + x;
            color = Vec3(image->data[index].r/255., image->data[index].g/255., image->data[index].b/255.);
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

void Material::set_texture(ppmLoader::ImageRGB *img) {
    image = img;
}

void Material::set_normals(ppmLoader::ImageRGB *img) {
    normals = img;
    has_normal_map = true;
}

void Material::get_normal(Vec3& normal, float u, float v, Vec3 &T, Vec3 &B) {
    if (!has_normal_map) {
        return;
    }
    int x, y, index;
    u = clamp(u, 0., 1.);
    v = 1. - clamp(v, 0., 1.);
    x = int(u * (normals->w - 1));
    y = int(v * (normals->h - 1));
    index = y * normals->w + x;
    // normal = Vec3(normals->data[index].r, normals->data[index].g, normals->data[index].b);
    // Map the normal values from [0, 255] to [-1, 1]
    Vec3 normal_from_map = Vec3(normals->data[index].r/127.5 - 1., normals->data[index].g/127.5 - 1., normals->data[index].b/127.5 - 1.);
    
    normal = normal_from_map[0] * T + normal_from_map[1] * B + normal_from_map[2] * normal;
    
    normal.normalize();
}