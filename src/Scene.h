#ifndef SCENE_H
#define SCENE_H

#include <vector>
#include <string>
#include "Material.h"
#include "Mesh.h"
#include "Sphere.h"
#include "Square.h"
#include "Vec3.h"

#include <cstdlib>
#include <ctime>
#include <iostream>

#include <GL/glut.h>

#include "Functions.h"
#include "imageLoader.h"

#define MAXBOUNCES 6
#define NB_ECH 10

enum LightType {
    LightType_Spherical,
    LightType_Quad
};


struct Light {
    Vec3 material;
    bool isInCamSpace;
    LightType type;

    Vec3 pos;
    float radius;

    Mesh quad;

    float powerCorrection;

    Light() : powerCorrection(1.0) {}

};

struct RaySceneIntersection{
    bool intersectionExists;
    unsigned int typeOfIntersectedObject;
    unsigned int objectIndex;
    float t;
    RayTriangleIntersection rayMeshIntersection; // 3
    RaySphereIntersection raySphereIntersection; // 1
    RaySquareIntersection raySquareIntersection; // 2
    RaySceneIntersection() : intersectionExists(false) , t(FLT_MAX) {}
};



class Scene {
    std::vector< Mesh > meshes;
    std::vector< Sphere > spheres;
    std::vector< Square > squares;
    std::vector< Light > lights;
    std::vector< ppmLoader::ImageRGB > textures;
    std::vector< ppmLoader::ImageRGB > normals;
    ppmLoader::ImageRGB skybox;

public:


    Scene() {
    }

    void draw() {
        // iterer sur l'ensemble des objets, et faire leur rendu :
        for( unsigned int It = 0 ; It < meshes.size() ; ++It ) {
            Mesh const & mesh = meshes[It];
            mesh.draw();
            if (mesh.kdtree) {
                mesh.kdtree->draw();
            }
        }
        for( unsigned int It = 0 ; It < spheres.size() ; ++It ) {
            Sphere const & sphere = spheres[It];
            sphere.draw();
        }
        for( unsigned int It = 0 ; It < squares.size() ; ++It ) {
            Square const & square = squares[It];
            square.draw();
        }
    }

    void addBox(std::vector<Material> const & materials, bool faces[6], Vec3 const & pos, Vec3 const rotation, float const size = 1.f, bool facing_out = true) {
        Vec3 base_bottom_left = Vec3(-size/2.);
        Vec3 base_right_vector = Vec3(size, 0., 0.);
        Vec3 base_up_vector = Vec3(0., 0., size);
        int nfaces = 0;
        if (faces[0]) { // bottom
            squares.resize(squares.size() + 1);
            Square & square = squares.back();
            square.setQuad(base_bottom_left, base_right_vector, base_up_vector, 1., 1.);
            nfaces++;
        }
        if (faces[1]) { // top
            squares.resize(squares.size() + 1);
            Square & square = squares.back();
            square.setQuad(base_bottom_left, base_right_vector, base_up_vector, 1., 1.);
            square.rotate_x(180.);
            nfaces++;
        }
        if (faces[2]){ // front
            squares.resize(squares.size() + 1);
            Square & square = squares.back();
            square.setQuad(base_bottom_left, base_right_vector, base_up_vector, 1., 1.);
            square.rotate_x(90.);
            nfaces++;
        }
        if (faces[3]) { // back
            squares.resize(squares.size() + 1);
            Square & square = squares.back();
            square.setQuad(base_bottom_left, base_right_vector, base_up_vector, 1., 1.);
            square.rotate_x(-90.);
            nfaces++;
        }
        if (faces[4]) { // left
            squares.resize(squares.size() + 1);
            Square & square = squares.back();
            square.setQuad(base_bottom_left, base_right_vector, base_up_vector, 1., 1.);
            square.rotate_x(90.);
            square.rotate_y(90.);
            nfaces++;
        }
        if (faces[5]) { // right
            squares.resize(squares.size() + 1);
            Square & square = squares.back();
            square.setQuad(base_bottom_left, base_right_vector, base_up_vector, 1., 1.);
            square.rotate_x(90.);
            square.rotate_y(-90.);
            nfaces++;
        }
        for (int i = squares.size() - nfaces; i < squares.size(); i++) {
            squares[i].translate(pos);
            squares[i].build_arrays();
            if (!facing_out) squares[i].m_normal *= -1.;
            squares[i].material = materials[i - squares.size() + nfaces];
        }
    }


    Vec3 skyboxTexture(Vec3 direction, int NRemainingBounces) {
        if (skybox.w < 1 || skybox.h < 1) {
            float a = 0.5*(direction[1] + 1.0);
            return (1.0-a)*Vec3(1.0, 1.0, 1.0) + a*Vec3(0.5, 0.7, 1.0) * (NRemainingBounces+1);
        }
        float u = 0.5 + atan2(direction[2], direction[0]) / (2 * M_PI);
        float v = 0.5 - asin(direction[1]) / M_PI;
        int x = u * skybox.w;
        int y = v * skybox.h;
        int index = y * skybox.w + x;
        // // average all the adjacent pixels (to avoid aliasing), compute the adjacent indices modulo the image size to handle the borders
        // Vec3 color = Vec3(0.);
        // int adjIndex;
        // for (int i = -1; i <= 1; i++) {
        //     for (int j = -1; j <= 1; j++) {
        //         adjIndex = (x + i + skybox.w) % skybox.w + (y + j + skybox.h) % skybox.h * skybox.w;
        //         color += Vec3(skybox.data[adjIndex].r/255., skybox.data[adjIndex].g/255., skybox.data[adjIndex].b/255.);
        //     }
        // }
        // color /= 9.;
        // color = Vec3(skybox.data[index].r/255., skybox.data[index].g/255., skybox.data[index].b/255.);
        return Vec3(skybox.data[index].r/255., skybox.data[index].g/255., skybox.data[index].b/255.) * NRemainingBounces;
    }

    void loadSkybox(const std::string &filename) {
        ppmLoader::load_ppm(skybox, filename);
    }

    int load_texture(const std::string &filename) {
        ppmLoader::ImageRGB img;
        ppmLoader::load_ppm(img, filename);
        textures.push_back(img);
        return textures.size() - 1;
    }

    int load_normal_map(const std::string &filename) {
        ppmLoader::ImageRGB img;
        ppmLoader::load_ppm(img, filename);
        normals.push_back(img);
        return normals.size() - 1;
    }

    void clear() {
        meshes.clear();
        spheres.clear();
        squares.clear();
        lights.clear();
        textures.clear();
        normals.clear();
    }

    void setResult(RaySceneIntersection &result, int typeOfIntersectedObject, int objectIndex, float t) {
        result.typeOfIntersectedObject = typeOfIntersectedObject;
        result.objectIndex = objectIndex;
        result.t = t;
        result.intersectionExists = true;
    }

    /**
     * Retourne l'intersection la plus proche
     * Si tmax n'est pas spécifié, on garde la plus proche intersection (rayon infini)
     * Si tmax est spécifié, on ne garde que les intersections avec t < tmax
     */
    RaySceneIntersection computeIntersection(Ray const & ray, float tmax = FLT_MAX) {
        RaySceneIntersection result;
        //TODO calculer les intersections avec les objets de la scene et garder la plus proche
        result.typeOfIntersectedObject = 0;
        result.objectIndex = -1;
        result.t = tmax;
        for (int i = 0; i < spheres.size(); i++) {
            RaySphereIntersection intersection = spheres[i].intersect(ray);
            if (intersection.intersectionExists && intersection.t < result.t && intersection.t >= EPSILON) {
                setResult(result, 1, i, intersection.t);
                result.raySphereIntersection = intersection;
            } 
        }
        // std::cout << "\nSphere intersection : " << result.raySphereIntersection.t << std::endl;
        // std::cout << "Sphere intersection exists : " << result.raySphereIntersection.intersectionExists << std::endl;
        for (int i = 0; i < squares.size(); i++) {
            RaySquareIntersection intersection = squares[i].intersect(ray);
            if (intersection.intersectionExists && intersection.t < result.t && intersection.t >= EPSILON) {
                setResult(result, 2, i, intersection.t);
                result.raySquareIntersection = intersection;
            } 
        }
        // std::cout << "Square intersection : " << result.raySquareIntersection.t << std::endl;
        // std::cout << "Square intersection exists : " << result.raySquareIntersection.intersectionExists << std::endl;
        for (int i = 0; i < meshes.size(); i++) {
            RayTriangleIntersection intersection = meshes[i].intersect(ray);
            if (intersection.intersectionExists && intersection.t < result.t && intersection.t >= EPSILON) {
                setResult(result, 3, i, intersection.t);
                result.rayMeshIntersection = intersection;  
            } 
        }
        // std::cout << "Mesh intersection : " << result.rayMeshIntersection.t << "\n" << std::endl;
        // std::cout << "Mesh intersection exists : " << result.rayMeshIntersection.intersectionExists << std::endl;
        return result;
    }

    /**
     * Retourne vrai si une intersection est trouvée avec un objet de la scène avant t
     */
    bool computeShadow(Ray const & ray, float t = FLT_MAX) {
        for (int i = 0; i < spheres.size(); i++) {
            RaySphereIntersection intersection = spheres[i].intersect(ray);
            if (intersection.intersectionExists && intersection.t < t && intersection.t >= EPSILON) return true;
        }
        for (int i = 0; i < squares.size(); i++) {
            RaySquareIntersection intersection = squares[i].intersect(ray);
            if (intersection.intersectionExists && intersection.t < t && intersection.t >= EPSILON) return true;
        }
        for (int i = 0; i < meshes.size(); i++) {
            RayTriangleIntersection intersection = meshes[i].intersect(ray);
            if (intersection.intersectionExists && intersection.t < t && intersection.t >= EPSILON) return true;
        }
        return false;
    }

    
    Vec3 rayTraceRecursive( Ray ray , int NRemainingBounces ) {
        Vec3 color = Vec3(0.f);
        if (NRemainingBounces == 0) return color;
        RaySceneIntersection raySceneIntersection = computeIntersection(ray);
        Material material;

        Vec3 normal;
        Vec3 intersection;
        Vec3 emission;
        Vec3 L, R, V;

       
        switch (raySceneIntersection.typeOfIntersectedObject) {
            case 1: // Sphere
                material = spheres[raySceneIntersection.objectIndex].material;
                intersection = raySceneIntersection.raySphereIntersection.intersection;
                normal = raySceneIntersection.raySphereIntersection.normal;
                material.sphere_texture(material.diffuse_material, raySceneIntersection.raySphereIntersection.phi, raySceneIntersection.raySphereIntersection.theta);
                //material.get_normal(normal, raySceneIntersection.raySphereIntersection.phi / (2 * M_PI), raySceneIntersection.raySphereIntersection.theta / M_PI);
                material.emit(emission, raySceneIntersection.raySphereIntersection.phi / (2 * M_PI), raySceneIntersection.raySphereIntersection.theta / M_PI);
                break;
            case 2: // Square
                material = squares[raySceneIntersection.objectIndex].material;
                intersection = raySceneIntersection.raySquareIntersection.intersection;
                normal = raySceneIntersection.raySquareIntersection.normal;
                material.texture(material.diffuse_material, raySceneIntersection.raySquareIntersection.u, raySceneIntersection.raySquareIntersection.v);
                material.get_normal(normal, raySceneIntersection.raySquareIntersection.u, raySceneIntersection.raySquareIntersection.v, squares[raySceneIntersection.objectIndex].m_right_vector, squares[raySceneIntersection.objectIndex].m_up_vector);
                material.emit(emission, raySceneIntersection.raySquareIntersection.u, raySceneIntersection.raySquareIntersection.v);
                break;
            case 3: // Mesh
                material = meshes[raySceneIntersection.objectIndex].material;
                intersection = raySceneIntersection.rayMeshIntersection.intersection;
                normal = raySceneIntersection.rayMeshIntersection.normal;
                if (meshes[raySceneIntersection.objectIndex].colorType == ColorType_Vertex) {
                    L = meshes[raySceneIntersection.objectIndex].vertColors[meshes[raySceneIntersection.objectIndex].triangles[raySceneIntersection.rayMeshIntersection.tIndex][0]];
                    R = meshes[raySceneIntersection.objectIndex].vertColors[meshes[raySceneIntersection.objectIndex].triangles[raySceneIntersection.rayMeshIntersection.tIndex][1]];
                    V = meshes[raySceneIntersection.objectIndex].vertColors[meshes[raySceneIntersection.objectIndex].triangles[raySceneIntersection.rayMeshIntersection.tIndex][2]];
                    material.diffuse_material = raySceneIntersection.rayMeshIntersection.w0 * L + raySceneIntersection.rayMeshIntersection.w1 * R + raySceneIntersection.rayMeshIntersection.w2 * V;
                } else if (meshes[raySceneIntersection.objectIndex].colorType == ColorType_Face) {
                    material.diffuse_material = meshes[raySceneIntersection.objectIndex].faceColors[raySceneIntersection.rayMeshIntersection.tIndex];
                }
                break;
            case 0: // No intersection
            default:
                return skyboxTexture(ray.direction(), NRemainingBounces);
                break;
        }
        for (int i = 0; i < lights.size(); i++) {
            L = lights[i].pos - intersection;
            L.normalize();
            float dotLN = Vec3::dot(L, normal);

            // Diffuse
            color += Vec3::compProduct(lights[0].material, material.diffuse_material) * max(0.0, dotLN) * (1. - material.transparency);

            // Specular
            R = 2.*(dotLN)*normal-L;
            R.normalize();
            V = ray.direction() * -1.;
            //color += Vec3::compProduct(lights[0].material, material.specular_material)  * pow(max(0.0, Vec3::dot(R, V)),material.shininess);
        
            // Ombres douces
            Light random_light;
            int blocked = 0;
            //int nb_ech = 10;
            int nb_ech = NB_ECH;
            float delta = lights[i].radius/2.;
            for (int j = 0; j < nb_ech; j++) {
                // float x = random_float(-delta, delta);
                // float y = 0.0;
                // float z = random_float(-delta, delta);
                // random_light.pos = lights[i].pos + Vec3(x, y, z);
                random_light.pos = lights[i].pos + random_unit_vector() * delta;
                L = random_light.pos - intersection;
                L.normalize();
                float tLight = (random_light.pos - intersection).length();
                if (computeShadow(Ray(intersection + L * EPSILON, L), tLight)) blocked++;
            }
            float shadow = 1. - float(blocked) / float(nb_ech);
            color *= shadow;
        }
        Vec3 newColor;
        Ray newRay;
        material.scatter(ray, normal, intersection, newRay);
        newColor = rayTraceRecursive(newRay, NRemainingBounces-1);
        newColor = Vec3::compProduct(newColor, material.diffuse_material);
        return color + newColor + emission;
    }


    Vec3 rayTrace( Ray const & rayStart ) {
        //TODO appeler la fonction recursive
        int bounces = MAXBOUNCES;
        Vec3 color = Vec3(0.) + rayTraceRecursive(rayStart, bounces);
        color /= (float)bounces;
        return color;
    }

    void computeKDTrees() {
        for (int i = 0; i < meshes.size(); i++) {
            meshes[i].computeKDTree();
        }
    }

    void setup_single_sphere() {
        clear();
        loadSkybox("img/textures/space.ppm");
        {
            lights.resize( lights.size() + 1 );
            Light & light = lights[lights.size() - 1];
            light.pos = Vec3(-5,5,5);
            light.radius = 2.5f;
            light.powerCorrection = 2.f;
            light.type = LightType_Spherical;
            light.material = Vec3(1,1,1);
            light.isInCamSpace = false;
        }
        {
            spheres.resize( spheres.size() + 1 );
            Sphere & s = spheres[spheres.size() - 1];
            s.m_center = Vec3(0. , 0. , 0.);
            s.m_radius = 1.f;
            s.build_arrays();
            s.material.type = Material_Mirror;
            s.material.diffuse_material = Vec3( 1. );
            s.material.specular_material = Vec3( 0.2,0.2,0.2 );
            s.material.shininess = 20;
        }
    }

    void setup_single_square() {
        clear();
        {
            lights.resize( lights.size() + 1 );
            Light & light = lights[lights.size() - 1];
            light.pos = Vec3(-5,5,5);
            light.radius = 2.5f;
            light.powerCorrection = 2.f;
            light.type = LightType_Spherical;
            light.material = Vec3(1,1,1);
            light.isInCamSpace = false;
        }

        {
            squares.resize( squares.size() + 1 );
            Square & s = squares[squares.size() - 1];
            s.setQuad(Vec3(-1., -1., 0.), Vec3(1., 0, 0.), Vec3(0., 1, 0.), 6., 2.);
            s.build_arrays();
            s.material.diffuse_material = Vec3( 1.,0.,0. );
            s.material.specular_material = Vec3( 0.8,0.8,0.8 );
            s.material.shininess = 20;
        }
        { //Right Wall
            squares.resize( squares.size() + 1 );
            Square & s = squares[squares.size() - 1];
            s.setQuad(Vec3(-1., -1., 0.), Vec3(1., 0, 0.), Vec3(0., 1, 0.), 2., 2.);
            s.translate(Vec3(0., 0., -2.));
            s.scale(Vec3(2., 2., 1.));
            s.rotate_y(-90);
            s.build_arrays();
            s.material.diffuse_material = Vec3( 0.0,1.0,0.0 );
            s.material.specular_material = Vec3( 0.0,1.0,0.0 );
            s.material.shininess = 16;
        }
    }

    void setup_cornell_box(float aspect_ratio) {
        clear();
        skybox = ppmLoader::ImageRGB();
        int brickwall_texture = load_texture("img/planeTextures/brickwall.ppm");
        int brickwall_normal = load_normal_map("img/normalMaps/brickwall_normal.ppm");
        int floor_normal = load_normal_map("img/normalMaps/n1.ppm");
        int sand_texture = load_texture("img/planeTextures/sand.ppm");

        // {
        //     lights.resize( lights.size() + 1 );
        //     Light & light = lights[lights.size() - 1];
        //     // base settings : 0.0    1.5      0.0
        //     light.pos = Vec3( 0.0, 1.5, 0.0 );
        //     light.radius = 1.5f;
        //     light.powerCorrection = 2.f; 
        //     light.type = LightType_Spherical;
        //     light.material = Vec3(1,1,1);
        //     light.isInCamSpace = false;
        // }

        // { //Ceiling square emissive light
        //     squares.resize( squares.size() + 1 );
        //     Square & s = squares[squares.size() - 1];
        //     s.setQuad(Vec3(-1., -1., 0.), Vec3(1., 0, 0.), Vec3(0., 1, 0.), 2., 2.);
        //     s.translate(Vec3(0., 0., -2.));
        //     s.scale(Vec3(0.5, 0.5, 1.));
        //     s.rotate_x(90);
        //     s.translate(Vec3(0., -0.00001, 0.));
        //     s.build_arrays();
        //     s.material.diffuse_material = Vec3( 1.0,1.0,1.0 );
        //     s.material.specular_material = Vec3( 1.0,1.0,1.0 );
        //     s.material.shininess = 16;
        //     s.material.emissive = true;
        //     s.material.light_color = Vec3(1.);
        //     s.material.light_intensity = 30.;
        // }

        // { //Light Sphere
        //     spheres.resize( spheres.size() + 1 );
        //     Sphere & s = spheres[spheres.size() - 1];
        //     s.m_center = Vec3(-0.5, 1.2, -1.5);
        //     s.m_radius = 0.25f;
        //     s.build_arrays();
        //     s.material.type = Material_Diffuse_Blinn_Phong; 
        //     s.material.diffuse_material = Vec3( 0.7 );
        //     s.material.specular_material = Vec3(  1.,1.,1. );
        //     s.material.shininess = 16;
        //     s.material.transparency = 0.;
        //     s.material.index_medium = 0.;
        //     s.material.emissive = true;
        //     s.material.light_color = Vec3(1.);
        //     s.material.light_intensity = 50.;
        // }

        std::vector<Material> materials;
        Material white = Material();
        white.diffuse_material = Vec3(0.9);
        white.specular_material = Vec3(1.);
        white.shininess = 16;
        Material emissive = Material();
        emissive.emissive = true;
        emissive.light_color = Vec3(1.);
        emissive.light_intensity = 30.;
        materials.push_back(emissive);
        for (int i = 0; i < 4; i++) {
            materials.push_back(white);
        }
        Vec3 pos = Vec3(0., 1.95, 0.);
        bool faces[6] = {true, false, true, true, true, true};
        addBox(materials, faces, pos, Vec3(45.), 1., false);

        { //Back Wall
            squares.resize( squares.size() + 1 );
            Square & s = squares[squares.size() - 1];
            s.setQuad(Vec3(-1., -1., 0.), Vec3(1., 0, 0.), Vec3(0., 1, 0.), 2., 2.);
            s.scale(Vec3(2.*aspect_ratio, 2., 1.));
            s.translate(Vec3(0., 0., -2.));
            s.build_arrays();
            s.material.diffuse_material = Vec3( 1.,1.,1. );
            s.material.specular_material = Vec3( 1.,1.,1. );
            s.material.shininess = 16;
            s.material.texture_type = Texture_Image;
            s.material.set_texture(&textures[brickwall_texture]);
            s.material.set_normals(&normals[brickwall_normal]);
            s.material.texture_scale_x = 1.*aspect_ratio;
            s.material.texture_scale_y = 1.;
        }

        // 1 = 9
        // x = 16

        { //Left Wall

            squares.resize( squares.size() + 1 );
            Square & s = squares[squares.size() - 1];
            s.setQuad(Vec3(-1., -1., 0.), Vec3(1., 0, 0.), Vec3(0., 1, 0.), 2., 2.);
            s.rotate_x(180);
            s.scale(Vec3(2., 2., 1.));
            s.translate(Vec3(0, 0., -2.*(-aspect_ratio)));
            s.rotate_y(90);
            s.build_arrays();
            s.material.diffuse_material = Vec3( 1.,0.,0. );
            s.material.specular_material = Vec3( 1.,0.,0. );
            s.material.shininess = 16;
            s.material.texture_type = Texture_Image;
            s.material.set_texture(&textures[brickwall_texture]);
            s.material.set_normals(&normals[brickwall_normal]);
        }

        { //Right Wall
            squares.resize( squares.size() + 1 );
            Square & s = squares[squares.size() - 1];
            s.setQuad(Vec3(-1., -1., 0.), Vec3(1., 0, 0.), Vec3(0., 1, 0.), 2., 2.);
            s.rotate_x(180);
            s.translate(Vec3(0., 0., -2.*(-aspect_ratio)));
            s.scale(Vec3(2., 2., 1.));
            s.rotate_y(-90);
            s.build_arrays();
            s.material.diffuse_material = Vec3( 0.0,1.0,0.0 );
            s.material.specular_material = Vec3( 0.0,1.0,0.0 );
            s.material.shininess = 16;
            s.material.texture_type = Texture_Image;
            s.material.set_texture(&textures[brickwall_texture]);
            s.material.set_normals(&normals[brickwall_normal]);
        }

        { //Floor
            squares.resize( squares.size() + 1 );
            Square & s = squares[squares.size() - 1];
            s.setQuad(Vec3(-1., -1., 0.), Vec3(1., 0, 0.), Vec3(0., 1, 0.), 2., 2.);
            s.translate(Vec3(0., 0., -2.));
            s.scale(Vec3(2.*aspect_ratio, 2., 1.));
            s.rotate_x(-90);
            s.build_arrays();
            s.material.diffuse_material = Vec3( 246./255., 204./255., 162./255. );
            s.material.specular_material = Vec3( 1.0,1.0,1.0 );
            s.material.shininess = 1;
            //s.material.texture_type = Texture_Image;
            //s.material.set_texture(&textures[sand_texture]);
            s.material.set_normals(&normals[floor_normal]);
        }

        { //Ceiling
            squares.resize( squares.size() + 1 );
            Square & s = squares[squares.size() - 1];
            s.setQuad(Vec3(-1., -1., 0.), Vec3(1., 0, 0.), Vec3(0., 1, 0.), 2., 2.);
            s.translate(Vec3(0., 0., -2.));
            s.scale(Vec3(2.*aspect_ratio, 2., 1.));
            s.rotate_x(90);
            s.build_arrays();
            s.material.diffuse_material = Vec3( 1.0,1.0,1.0 );
            s.material.specular_material = Vec3( 1.0,1.0,1.0 );
            s.material.shininess = 16;
            s.material.texture_type = Texture_Checkerboard;
            s.material.checkerboard_color1 = Vec3(0.95);
            s.material.checkerboard_color2 = Vec3(0.5);
            s.material.texture_scale_x = 8.*aspect_ratio;
            s.material.texture_scale_y = 8.;
        }
       
       
  
        { //Front Wall
            squares.resize( squares.size() + 1 );
            Square & s = squares[squares.size() - 1];
            s.setQuad(Vec3(-1., -1., 0.), Vec3(1., 0, 0.), Vec3(0., 1, 0.), 2., 2.);
            s.translate(Vec3(0., 0., -2.));
            s.scale(Vec3(2.*aspect_ratio, 2., 1.));
            s.rotate_y(180);
            s.build_arrays();
            s.material.diffuse_material = Vec3( 1.0,1.0,1.0 );
            s.material.specular_material = Vec3( 1.0,1.0,1.0 );
            s.material.shininess = 16;
            s.material.texture_type = Texture_Image;
            s.material.set_texture(&textures[brickwall_texture]);
            s.material.set_normals(&normals[brickwall_normal]);
        }


        { //GLASS Sphere

            spheres.resize( spheres.size() + 1 );
            Sphere & s = spheres[spheres.size() - 1];
            s.m_center = Vec3(1.0, -1.25, 0.5);
            s.m_radius = 0.75f;
            s.build_arrays();
            s.material.type = Material_Glass;
            s.material.diffuse_material = Vec3( 1.);
            s.material.specular_material = Vec3( 1.);
            s.material.shininess = 16;
            s.material.transparency = 1.0;
            s.material.index_medium = 1.4;
        }


        { //MIRRORED Sphere
            spheres.resize( spheres.size() + 1 );
            Sphere & s = spheres[spheres.size() - 1];
            s.m_center = Vec3(-1.0, -1.25, -0.5);
            s.m_radius = 0.75f;
            s.build_arrays();
            s.material.type = Material_Mirror; 
            s.material.diffuse_material = Vec3( 0.7 );
            s.material.specular_material = Vec3(  1.,1.,1. );
            s.material.shininess = 16;
            s.material.transparency = 0.;
            s.material.index_medium = 0.;
        }

        // {
        //     meshes.resize( meshes.size() + 1 );
        //     Mesh & m = meshes[meshes.size() - 1];
        //     m.loadOFF("mesh/tripod.off");
        //     m.translate(Vec3(0., 0., 0.));
        //     m.build_arrays();
        //     m.material.type = Material_Glass;
        //     m.material.index_medium = 1.333;
        //     m.material.transparency = 0.9;
        //     m.material.diffuse_material = Vec3( 0.1,0.2, 0.5);
        //     m.material.specular_material = Vec3( 0.9, 0.9, 0.9 );
        //     m.material.shininess = 32;
        // }
    
    }

    void setup_rt_in_a_weekend() {
        clear();
        loadSkybox("img/textures/sky.ppm");
        int sun_texture = load_texture("img/sphereTextures/s2.ppm");
        {
            lights.resize( lights.size() + 1 );
            Light & light = lights[lights.size() - 1];
            light.pos = Vec3( 0.0, 3., -8.0 );
            light.radius = 1.5f;
            light.powerCorrection = 2.f;
            light.type = LightType_Spherical;
            light.material = Vec3(1,1,1);
            light.isInCamSpace = false;
        }
        {
            lights.resize( lights.size() + 1 );
            Light & light = lights[lights.size() - 1];
            light.pos = Vec3( -4., 3., -8.0 );
            light.radius = 1.5f;
            light.powerCorrection = 2.f;
            light.type = LightType_Spherical;
            light.material = Vec3(1,1,1);
            light.isInCamSpace = false;
        }
        {
            lights.resize( lights.size() + 1 );
            Light & light = lights[lights.size() - 1];
            light.pos = Vec3( 4., 3., -8.0 );
            light.radius = 1.5f;
            light.powerCorrection = 2.f;
            light.type = LightType_Spherical;
            light.material = Vec3(1,1,1);
            light.isInCamSpace = false;
        }

        { // Glass Sphere
            spheres.resize( spheres.size() + 1 );
            Sphere & s = spheres[spheres.size() - 1];
            s.m_center = Vec3(-4. , 0. , -8.);
            s.m_radius = 2.f;
            s.build_arrays();
            s.material.type = Material_Glass;
            s.material.diffuse_material = Vec3( 0.8 );
            s.material.specular_material = Vec3( 0.8 );
            s.material.index_medium = 1.5;
            s.material.shininess = 20;
        }
        { // Diffuse Sphere
            spheres.resize( spheres.size() + 1 );
            Sphere & s = spheres[spheres.size() - 1];
            s.m_center = Vec3(0. , 0. , -8.);
            s.m_radius = 2.f;
            s.build_arrays();
            s.material.diffuse_material = Vec3( 0.1,0.2, 0.5);
            s.material.specular_material = Vec3( 0.2,0.2,0.2 );
            s.material.shininess = 20;
            s.material.texture_type = Texture_Image;
            s.material.set_texture(&textures[sun_texture]);
            s.material.emissive = true;
            s.material.light_intensity = 15.;
        }
        { // Mirror Sphere
            spheres.resize( spheres.size() + 1 );
            Sphere & s = spheres[spheres.size() - 1];
            s.m_center = Vec3(4. , 0. , -8.);
            s.m_radius = 2.f;
            s.build_arrays();
            s.material.type = Material_Mirror;
            s.material.diffuse_material = Vec3( 0.8 );
            s.material.specular_material = Vec3( 0.8 );
            s.material.shininess = 32;
        }

        { //Floor
            squares.resize( squares.size() + 1 );
            Square & s = squares[squares.size() - 1];
            s.setQuad(Vec3(-1., -0.2, 0.), Vec3(1., 0, 0.), Vec3(0., 1, 0.), 2., 2.);
            s.translate(Vec3(0., 0., -2.));
            s.scale(Vec3(50., 50., 1.));
            s.rotate_x(-90);
            s.build_arrays();
            s.material.diffuse_material = Vec3( 0.1,0.2,0.5 );
            s.material.specular_material = Vec3( 1.0,1.0,1.0 );
            s.material.shininess = 16;
            s.material.texture_type = Texture_Checkerboard;
            s.material.checkerboard_color1 = Vec3(1.);
            s.material.checkerboard_color2 = Vec3( 0.1,0.2,0.5 );
            s.material.texture_scale_x = 100.;
s.material.texture_scale_y = 100.;
        }
    }

    void setup_mesh() {
        clear();
        loadSkybox("img/textures/space.ppm");

        {
            lights.resize( lights.size() + 1 );
            Light & light = lights[lights.size() - 1];
            light.pos = Vec3( 0.0, 3., 2.0 );
            light.radius = 1.5f;
            light.powerCorrection = 2.f;
            light.type = LightType_Spherical;
            light.material = Vec3(1,1,1);
            light.isInCamSpace = false;
        }

        { // Diffuse Sphere
            spheres.resize( spheres.size() + 1 );
            Sphere & s = spheres[spheres.size() - 1];
            s.m_center = Vec3(0. , 0. , -16.);
            s.m_radius = 2.f;
            s.build_arrays();
            s.material.diffuse_material = Vec3( 0.1 , 0.6, 0.2);
            s.material.specular_material = Vec3( 0.1 , 0.6, 0.2);
            s.material.shininess = 20;
        }

        { // Mirror Sphere
            spheres.resize( spheres.size() + 1 );
            Sphere & s = spheres[spheres.size() - 1];
            s.m_center = Vec3(4. , 0. , -8.);
            s.m_radius = 2.f;
            s.build_arrays();
            s.material.type = Material_Mirror;
            s.material.diffuse_material = Vec3( 0.8 );
            s.material.specular_material = Vec3( 0.8 );
            s.material.shininess = 32;
        }

        {
            meshes.resize( meshes.size() + 1 );
            Mesh & m = meshes[meshes.size() - 1];
            m.loadOFF("mesh/blob-closed.off");
            m.translate(Vec3(0., 0.9, -4.));
            m.scale(Vec3(1.5));
            m.rotate_x(180);
            m.rotate_y(180);
            m.build_arrays();
            m.material.type = Material_Glass;
            m.material.index_medium = 1.333;
            m.material.transparency = 0.9;
            m.material.diffuse_material = Vec3( 0.1,0.2, 0.5);
            m.material.specular_material = Vec3( 0.9, 0.9, 0.9 );
            m.material.shininess = 32;
        }

        { // Eye 1
            spheres.resize( spheres.size() + 1 );
            Sphere & s = spheres[spheres.size() - 1];
            s.m_center = Vec3(0.2, -1. , -4.8);
            s.m_radius = 0.3f;
            s.build_arrays();
            s.material.diffuse_material = Vec3( 1.);
            s.material.specular_material = Vec3( 1. );
            s.material.shininess = 20;
        }

        { // Pupil 1
            spheres.resize( spheres.size() + 1 );
            Sphere & s = spheres[spheres.size() - 1];
            s.m_center = Vec3(0.2, -1. , -4.55);
            s.m_radius = 0.1f;
            s.build_arrays();
            s.material.diffuse_material = Vec3( 0.);
            s.material.specular_material = Vec3( 1. );
            s.material.shininess = 20;
        }

        { // Eye 2
            spheres.resize( spheres.size() + 1 );
            Sphere & s = spheres[spheres.size() - 1];
            s.m_center = Vec3(-0.7 , -1. , -4.95);
            s.m_radius = 0.3f;
            s.build_arrays();
            s.material.diffuse_material = Vec3( 1.);
            s.material.specular_material = Vec3( 1. );
            s.material.shininess = 20;
        }

        { // Pupil 2
            spheres.resize( spheres.size() + 1 );
            Sphere & s = spheres[spheres.size() - 1];
            s.m_center = Vec3(-0.7 , -1. , -4.7);
            s.m_radius = 0.1f;
            s.build_arrays();
            s.material.diffuse_material = Vec3( 0.);
            s.material.specular_material = Vec3( 1. );
            s.material.shininess = 20;
        }


        { //Floor
            squares.resize( squares.size() + 1 );
            Square & s = squares[squares.size() - 1];
            s.setQuad(Vec3(-1., -0.2, 0.), Vec3(1., 0, 0.), Vec3(0., 1, 0.), 2., 2.);
            s.translate(Vec3(0., 0., -2.));
            s.scale(Vec3(50., 50., 1.));
            s.rotate_x(-90);
            s.build_arrays();
            s.material.diffuse_material = Vec3( 0.8,0.8,0. );
            s.material.specular_material = Vec3( 1.0,1.0,1.0 );
            s.material.shininess = 16;
        }
        std::cout << "Building KDTree" << std::endl;
        computeKDTrees();
    }

    void setup_random_spheres() {
        clear();
        int nSpheres = 79;
        {
            lights.resize( lights.size() + 1 );
            Light & light = lights[lights.size() - 1];
            light.pos = Vec3( -1.0, 8., 2.0 );
            light.radius = 1.5f;
            light.powerCorrection = 2.f;
            light.type = LightType_Spherical;
            light.material = Vec3(1,1,1);
            light.isInCamSpace = false;
        }

        { //Floor
            squares.resize( squares.size() + 1 );
            Square & s = squares[squares.size() - 1];
            s.setQuad(Vec3(-1., -0.2, 0.), Vec3(1., 0, 0.), Vec3(0., 1, 0.), 2., 2.);
            s.translate(Vec3(0., 0., -4.));
            s.scale(Vec3(100., 100., 1.));
            s.rotate_x(-90);
            s.build_arrays();
            s.material.diffuse_material = Vec3( 0.8,0.8,0. );
            s.material.specular_material = Vec3( 1.0,1.0,1.0 );
        }

        { // Mirror Sphere
            spheres.resize( spheres.size() + 1 );
            Sphere & s = spheres[spheres.size() - 1];
            s.m_center = Vec3(-3. , 0. , -22.);
            s.m_radius = 4.f;
            s.build_arrays();
            s.material.type = Material_Mirror;
            s.material.diffuse_material = Vec3( 0.8 );
            s.material.specular_material = Vec3( 0.8 );
            s.material.shininess = 32;
        }

        { // Mirror Sphere
            spheres.resize( spheres.size() + 1 );
            Sphere & s = spheres[spheres.size() - 1];
            s.m_center = Vec3(4. , -2. , -15.);
            s.m_radius = 2.f;
            s.build_arrays();
            s.material.type = Material_Mirror;
            s.material.diffuse_material = Vec3( 0.8 );
            s.material.specular_material = Vec3( 0.8 );
            s.material.shininess = 32;
        }

        { // Glass Sphere
            spheres.resize( spheres.size() + 1 );
            Sphere & s = spheres[spheres.size() - 1];
            s.m_center = Vec3(-1. , -2.5 , -8.);
            s.m_radius = 1.5f;
            s.build_arrays();
            s.material.type = Material_Glass;
            s.material.diffuse_material = Vec3( 0.8 );
            s.material.specular_material = Vec3( 0.8 );
            s.material.shininess = 20;
        }

        for (int i = 0; i < nSpheres; i++) {
            float radius = random_float(0.25, 1.5);
            int type = rand() % 3;
            spheres.resize( spheres.size() + 1 );
            Sphere & s = spheres[spheres.size() - 1];
            s.m_center = Vec3(random_float(-30., 30.), -4+radius, random_float(-50., -2.));
            s.m_radius = radius;
            s.build_arrays();
            switch (type) {
                case 0:
                    s.material.type = Material_Mirror;
                    s.material.diffuse_material = Vec3( random_float(0., 1.), random_float(0., 1.), random_float(0., 1.));
                    s.material.specular_material = Vec3( random_float(0., 1.), random_float(0., 1.), random_float(0., 1.));
                    s.material.shininess = random_float(32., 100.);
                    break;
                case 1:
                    s.material.type = Material_Glass;
                    s.material.diffuse_material = Vec3( random_float(0.7, 1.));
                    s.material.specular_material = Vec3( random_float(0.7, 1.));
                    s.material.shininess = random_float(32., 70.);
                    s.material.transparency = random_float(0.7, 1.);
                    s.material.index_medium = random_float(1., 2.);
                    break;
                default:
                    s.material.diffuse_material = Vec3( random_float(0., 1.), random_float(0., 1.), random_float(0., 1.));
                    s.material.specular_material = Vec3( random_float(0., 1.), random_float(0., 1.), random_float(0., 1.));
                    s.material.shininess = random_float(0., 30.);
                    break;
            }
        }
    }

    void setup_debug_refraction() {
        clear();
        {
            lights.resize( lights.size() + 1 );
            Light & light = lights[lights.size() - 1];
            light.pos = Vec3( -1.0, 8., 2.0 );
            light.radius = 1.5f;
            light.powerCorrection = 2.f;
            light.type = LightType_Spherical;
            light.material = Vec3(1,1,1);
            light.isInCamSpace = false;
        }
        { //Back Wall
            squares.resize( squares.size() + 1 );
            Square & s = squares[squares.size() - 1];
            s.setQuad(Vec3(-1., -1., 0.), Vec3(1., 0, 0.), Vec3(0., 1, 0.), 2., 2.);
            s.scale(Vec3(2., 2., 1.));
            s.translate(Vec3(-2., 2., -2.));
            s.build_arrays();
            s.material.diffuse_material = Vec3( 1.,0.,0. );
            s.material.specular_material = Vec3( 1.,1.,1. );
            s.material.shininess = 16;
        }
        { //Back Wall
            squares.resize( squares.size() + 1 );
            Square & s = squares[squares.size() - 1];
            s.setQuad(Vec3(-1., -1., 0.), Vec3(1., 0, 0.), Vec3(0., 1, 0.), 2., 2.);
            s.scale(Vec3(2., 2., 1.));
            s.translate(Vec3(-2., -2., -2.));
            s.build_arrays();
            s.material.diffuse_material = Vec3( 0.,1.,0. );
            s.material.specular_material = Vec3( 1.,1.,1. );
            s.material.shininess = 16;
        }
        { //Back Wall
            squares.resize( squares.size() + 1 );
            Square & s = squares[squares.size() - 1];
            s.setQuad(Vec3(-1., -1., 0.), Vec3(1., 0, 0.), Vec3(0., 1, 0.), 2., 2.);
            s.scale(Vec3(2., 2., 1.));
            s.translate(Vec3(2., 2., -2.));
            s.build_arrays();
            s.material.diffuse_material = Vec3( 0.,0.,1. );
            s.material.specular_material = Vec3( 1.,1.,1. );
            s.material.shininess = 16;
        }
        { //Back Wall
            squares.resize( squares.size() + 1 );
            Square & s = squares[squares.size() - 1];
            s.setQuad(Vec3(-1., -1., 0.), Vec3(1., 0, 0.), Vec3(0., 1, 0.), 2., 2.);
            s.scale(Vec3(2., 2., 1.));
            s.translate(Vec3(2., -2., -2.));
            s.build_arrays();
            s.material.diffuse_material = Vec3( 1.,1.,1. );
            s.material.specular_material = Vec3( 1.,1.,1. );
            s.material.shininess = 16;
        }
        
        { //GLASS Sphere

            spheres.resize( spheres.size() + 1 );
            Sphere & s = spheres[spheres.size() - 1];
            s.m_center = Vec3(0., 0., 0.);
            s.m_radius = 0.75f;
            s.build_arrays();
            s.material.type = Material_Glass;
            s.material.diffuse_material = Vec3( 1.);
            s.material.specular_material = Vec3( 1.);
            s.material.shininess = 16;
            s.material.transparency = 1.0;
            s.material.index_medium = 1.4;
        }
    }

    // Starting to thing i may have too many scenes !
    void setup_flamingo() {
        clear();
        {
            lights.resize( lights.size() + 1 );
            Light & light = lights[lights.size() - 1];
            light.pos = Vec3( -1.0, 8., 2.0 );
            light.radius = 1.5f;
            light.powerCorrection = 2.f;
            light.type = LightType_Spherical;
            light.material = Vec3(1,1,1);
            light.isInCamSpace = false;
        }
        {
            lights.resize( lights.size() + 1 );
            Light & light = lights[lights.size() - 1];
            light.pos = Vec3( 1.0, 8., 2.0 );
            light.radius = 1.5f;
            light.powerCorrection = 2.f;
            light.type = LightType_Spherical;
            light.material = Vec3(1,1,1);
            light.isInCamSpace = false;
        }
        { //Floor
            squares.resize( squares.size() + 1 );
            Square & s = squares[squares.size() - 1];
            s.setQuad(Vec3(-1., -0.2, 0.), Vec3(1., 0, 0.), Vec3(0., 1, 0.), 2., 2.);
            s.translate(Vec3(0., 0., -2.));
            s.scale(Vec3(50., 50., 1.));
            s.rotate_x(-90);
            s.build_arrays();
            s.material.diffuse_material = Vec3( 0.8,0.8,0.  );
            s.material.specular_material = Vec3( 1.0,1.0,1.0 );
            s.material.shininess = 16;
            s.material.texture_type = Texture_Checkerboard;
            s.material.checkerboard_color1 = Vec3(0.8, 0.8, 0.);
            s.material.checkerboard_color2 = Vec3( 0.6,0.6,0.  );
            s.material.texture_scale_x = 100.;
s.material.texture_scale_y = 100.;
        }
        { // Glass Sphere
            spheres.resize( spheres.size() + 1 );
            Sphere & s = spheres[spheres.size() - 1];
            s.m_center = Vec3(-4. , 0. , -8.);
            s.m_radius = 2.f;
            s.build_arrays();
            s.material.type = Material_Glass;
            s.material.diffuse_material = Vec3( 0.8 );
            s.material.specular_material = Vec3( 0.8 );
            s.material.index_medium = 1.5;
            s.material.shininess = 20;
        }
        { // Mirror Sphere
            spheres.resize( spheres.size() + 1 );
            Sphere & s = spheres[spheres.size() - 1];
            s.m_center = Vec3(4. , 0. , -8.);
            s.m_radius = 2.f;
            s.build_arrays();
            s.material.type = Material_Mirror;
            s.material.diffuse_material = Vec3( 0.8 );
            s.material.specular_material = Vec3( 0.8 );
            s.material.shininess = 32;
        }
        {
            meshes.resize( meshes.size() + 1 );
            Mesh & m = meshes[meshes.size() - 1];
            m.loadOFF("mesh/flamingo_lowpoly_colored.off");
            m.scale(Vec3(2.5));
            m.rotate_x(90);
            m.rotate_y(90);
            m.rotate_z(180);
            m.translate(Vec3(0., 1., -8.));
            m.build_arrays();
            m.material.diffuse_material = Vec3( 0.1,0.2, 0.5);
            m.material.specular_material = Vec3( 0.9, 0.9, 0.9 );
            m.material.shininess = 6.;
        }
        computeKDTrees();
    }

    void setup_raccoon() {
        clear();
        loadSkybox("img/textures/sky.ppm");
        int fire_orb_texture = load_texture("img/sphereTextures/s2.ppm");
        int wind_orb_texture = load_texture("img/sphereTextures/s4.ppm");
        int water_orb_texture = load_texture("img/sphereTextures/s7.ppm");
        {
            lights.resize( lights.size() + 1 );
            Light & light = lights[lights.size() - 1];
            light.pos = Vec3( -1.0, 8., 2.0 );
            light.radius = 1.5f;
            light.powerCorrection = 2.f;
            light.type = LightType_Spherical;
            light.material = Vec3(1,1,1);
            light.isInCamSpace = false;
        }
        { //Flying carpet checker part
            squares.resize( squares.size() + 1 );
            Square & s = squares[squares.size() - 1];
            s.setQuad(Vec3(-1., -0.2, 0.), Vec3(1., 0, 0.), Vec3(0., 1, 0.), 2., 2.);
            s.translate(Vec3(0., 0., -2.));
            s.scale(Vec3(2., 4., 1.));
            s.rotate_x(-90);
            s.translate(Vec3(0., 0., -4.));
            s.build_arrays();
            s.material.diffuse_material = Vec3( 0.5,0.,0.5  );
            s.material.specular_material = Vec3( 1.0,1.0,1.0 );
            s.material.shininess = 4;
            s.material.texture_type = Texture_Checkerboard;
            s.material.checkerboard_color1 = Vec3(0.5, 0., 0.5);
            s.material.checkerboard_color2 = Vec3( 0.6,0.,0.6  );
            s.material.texture_scale_x = 16.;
s.material.texture_scale_y = 16.;
        }
        { //Flying carpet red part
            squares.resize( squares.size() + 1 );
            Square & s = squares[squares.size() - 1];
            s.setQuad(Vec3(-1., -0.2, 0.), Vec3(1., 0, 0.), Vec3(0., 1, 0.), 2., 2.);
            s.translate(Vec3(0., 0., -2.));
            s.scale(Vec3(2.5, 5., 1.));
            s.rotate_x(-90);
            s.translate(Vec3(0., -0.0001, -3.5));
            s.build_arrays();
            s.material.diffuse_material = Vec3( 0.9, 0.2, 0.);
            s.material.specular_material = Vec3( 1.0,1.0,1.0 );
            s.material.shininess = 4;
        }
        { // Raccoon
            meshes.resize( meshes.size() + 1 );
            Mesh & m = meshes[meshes.size() - 1];
            m.loadOFF("mesh/raccoon_low_poly_colored.off");
            m.rotate_y(-90);
            m.scale(Vec3(2.));
            m.translate(Vec3(0., -2., -5.));
            m.build_arrays();
            m.material.diffuse_material = Vec3( 0.1,0.2, 0.5);
            m.material.specular_material = Vec3( 0.9, 0.9, 0.9 );
            m.material.shininess = 6.;
        }
        {   // Staff
            meshes.resize( meshes.size() + 1 );
            Mesh & m = meshes[meshes.size() - 1];
            m.loadOFF("mesh/magic_staff_low_poly_colored.off");
            m.rotate_y(-90);
            m.rotate_z(90);
            m.scale(Vec3(0.15));
            m.translate(Vec3(1., 0.2, -2.7));
            m.build_arrays();
            m.material.diffuse_material = Vec3( 0.1,0.2, 0.5);
            m.material.specular_material = Vec3( 0.9, 0.9, 0.9 );
            m.material.shininess = 6.;
        }
        { // Staff orb
            spheres.resize( spheres.size() + 1 );
            Sphere & s = spheres[spheres.size() - 1];
            s.m_center = Vec3(-1.85 , 0.35 , -2.7);
            s.m_radius = 0.14f;
            s.build_arrays();
            s.material.type = Material_Glass;
            s.material.diffuse_material = Vec3( 0.451, 0.6627, 0.7608 );
            s.material.specular_material = Vec3( 1. );
            s.material.index_medium = 1.5;
            s.material.shininess = 64;
            s.material.transparency = 0.65;
        }
        { // Fire orb
            spheres.resize( spheres.size() + 1 );
            Sphere & s = spheres[spheres.size() - 1];
            s.m_center = Vec3(4. , 3. , -8.);
            s.m_radius = 1.3f;
            s.build_arrays();
            s.material.type = Material_Mirror;
            s.material.diffuse_material = Vec3( 0.8 , 0., 0.);
            s.material.specular_material = Vec3( 0.8 );
            s.material.shininess = 32;
            s.material.texture_type = Texture_Image;
            s.material.set_texture(&textures[fire_orb_texture]);
        }
        { // Wind orb
            spheres.resize( spheres.size() + 1 );
            Sphere & s = spheres[spheres.size() - 1];
            s.m_center = Vec3(-4. , 2. , -5.);
            s.m_radius = 0.9f;
            s.build_arrays();
            s.material.type = Material_Glass;
            s.material.diffuse_material = Vec3( 1.);
            s.material.specular_material = Vec3( 0.8 );
            s.material.shininess = 32;
            s.material.transparency = 0.4;
            s.material.texture_type = Texture_Image;
            s.material.set_texture(&textures[wind_orb_texture]);
        }
        { // Water orb
            spheres.resize( spheres.size() + 1 );
            Sphere & s = spheres[spheres.size() - 1];
            s.m_center = Vec3(-0.2 , 3. , -1.);
            s.m_radius = 1.4f;
            s.build_arrays();
            s.material.type = Material_Glass;
            s.material.diffuse_material = Vec3(0.5, 0.53, 0.8);
            s.material.specular_material = Vec3( 0.8 );
            s.material.shininess = 32;
            s.material.transparency = 0.8;
            s.material.texture_type = Texture_Image;
            s.material.set_texture(&textures[water_orb_texture]);
        }
        computeKDTrees();
    }

    void setup_flamingo_pond() {
        clear();
        loadSkybox("img/textures/sky.ppm");
        {
            lights.resize( lights.size() + 1 );
            Light & light = lights[lights.size() - 1];
            light.pos = Vec3( -1.0, 8., -19.0 );
            light.radius = 1.5f;
            light.powerCorrection = 2.f;
            light.type = LightType_Spherical;
            light.material = Vec3(1,1,1);
            light.isInCamSpace = false;
        }
        { // Pond. Note : when i'll implement acceleration structures, i should break this mesh into smaller parts
            meshes.resize( meshes.size() + 1 );
            Mesh & m = meshes[meshes.size() - 1];
            m.loadOFF("mesh/pond.off");
            m.scale(Vec3(3.));
            m.translate(Vec3(1., -5., -3.));
            m.build_arrays();
            m.material.diffuse_material = Vec3( 0.1,0.2, 0.5);
            m.material.specular_material = Vec3( 0.9, 0.9, 0.9 );
            m.material.shininess = 6.;
        }
        { // Water
            squares.resize( squares.size() + 1 );
            Square & s = squares[squares.size() - 1];
            s.setQuad(Vec3(-1., -0.2, 0.), Vec3(1., 0, 0.), Vec3(0., 1, 0.), 2., 2.);
            s.translate(Vec3(0., 0., -2.));
            s.scale(Vec3(5., 3.5, 1.));
            s.rotate_x(-90);
            s.translate(Vec3(1., 0., 2.8));
            s.build_arrays();
            s.material.diffuse_material = Vec3( 0.5,0.53,0.8 );
            s.material.specular_material = Vec3( 1.0,1.0,1.0 );
            s.material.shininess = 4;
            s.material.type = Material_Mirror;
        }
        { // Flamingo
            meshes.resize( meshes.size() + 1 );
            Mesh & m = meshes[meshes.size() - 1];
            m.loadOFF("mesh/flamingo_lowpoly_colored.off");
            m.scale(Vec3(0.8));
            m.rotate_x(90);
            m.rotate_y(115);
            m.rotate_z(180);
            m.translate(Vec3(3., -1.2, -1.));
            m.build_arrays();
            m.material.diffuse_material = Vec3( 0.1,0.2, 0.5);
            m.material.specular_material = Vec3( 0.9, 0.9, 0.9 );
            m.material.shininess = 6.;
        }
        computeKDTrees();
    }
};



#endif
