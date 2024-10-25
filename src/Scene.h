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

#define MAXBOUNCES 5

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

public:


    Scene() {
    }

    void draw() {
        // iterer sur l'ensemble des objets, et faire leur rendu :
        for( unsigned int It = 0 ; It < meshes.size() ; ++It ) {
            Mesh const & mesh = meshes[It];
            mesh.draw();
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
        for (int i = 0; i < squares.size(); i++) {
            RaySquareIntersection intersection = squares[i].intersect(ray);
            if (intersection.intersectionExists && intersection.t < result.t && intersection.t >= EPSILON) {
                setResult(result, 2, i, intersection.t);
                result.raySquareIntersection = intersection;
            } 
        }
        for (int i = 0; i < meshes.size(); i++) {
            RayTriangleIntersection intersection = meshes[i].intersect(ray);
            if (intersection.intersectionExists && intersection.t < result.t && intersection.t >= EPSILON) {
                setResult(result, 3, i, intersection.t);
                result.rayMeshIntersection = intersection;  
            } 
        }
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
        Vec3 L, R, V;

       
        switch (raySceneIntersection.typeOfIntersectedObject) {
            case 1: // Sphere
                material = spheres[raySceneIntersection.objectIndex].material;
                intersection = raySceneIntersection.raySphereIntersection.intersection;
                normal = raySceneIntersection.raySphereIntersection.normal;
                material.sphere_texture(material.diffuse_material, raySceneIntersection.raySphereIntersection.phi, raySceneIntersection.raySphereIntersection.theta);
                break;
            case 2: // Square
                material = squares[raySceneIntersection.objectIndex].material;
                intersection = raySceneIntersection.raySquareIntersection.intersection;
                normal = raySceneIntersection.raySquareIntersection.normal;
                material.texture(material.diffuse_material, raySceneIntersection.raySquareIntersection.u, raySceneIntersection.raySquareIntersection.v);
                break;
            case 3: // Mesh
                material = meshes[raySceneIntersection.objectIndex].material;
                intersection = raySceneIntersection.rayMeshIntersection.intersection;
                normal = raySceneIntersection.rayMeshIntersection.normal;
                L = meshes[raySceneIntersection.objectIndex].vertices[meshes[raySceneIntersection.objectIndex].triangles[raySceneIntersection.rayMeshIntersection.tIndex][0]].color;
                R = meshes[raySceneIntersection.objectIndex].vertices[meshes[raySceneIntersection.objectIndex].triangles[raySceneIntersection.rayMeshIntersection.tIndex][1]].color;
                V = meshes[raySceneIntersection.objectIndex].vertices[meshes[raySceneIntersection.objectIndex].triangles[raySceneIntersection.rayMeshIntersection.tIndex][2]].color;
                material.diffuse_material = raySceneIntersection.rayMeshIntersection.w0 * L + raySceneIntersection.rayMeshIntersection.w1 * R + raySceneIntersection.rayMeshIntersection.w2 * V;
                break;
            case 0: // No intersection
            default:
                float a = 0.5*(ray.direction()[1] + 1.0);
                return (1.0-a)*Vec3(1.0, 1.0, 1.0) + a*Vec3(0.5, 0.7, 1.0) * NRemainingBounces;
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
            color += Vec3::compProduct(lights[0].material, material.specular_material)  * pow(max(0.0, Vec3::dot(R, V)),material.shininess);
        
            // Ombres douces
            Light random_light;
            int blocked = 0;
            int nb_ech = 10;
            float delta = lights[i].radius/2.;
            for (int j = 0; j < nb_ech; j++) {
                float x = random_float(-delta, delta);
                float y = 0.0;
                float z = random_float(-delta, delta);
                random_light.pos = lights[i].pos + Vec3(x, y, z);
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
        return color + newColor;
    }


    Vec3 rayTrace( Ray const & rayStart ) {
        //TODO appeler la fonction recursive
        int bounces = MAXBOUNCES;
        Vec3 color = Vec3(0.) + rayTraceRecursive(rayStart, bounces);
        color /= (float)bounces;
        return color;
    }

    void setup_single_sphere() {
        meshes.clear();
        spheres.clear();
        squares.clear();
        lights.clear();

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
            s.m_center = Vec3(-1. , 0. , 0.);
            s.m_radius = 1.f;
            s.build_arrays();
            s.material.type = Material_Mirror;
            s.material.diffuse_material = Vec3( 1.,0.,0. );
            s.material.specular_material = Vec3( 0.2,0.2,0.2 );
            s.material.shininess = 20;
        }
        {
            spheres.resize( spheres.size() + 1 );
            Sphere & s = spheres[spheres.size() - 1];
            s.m_center = Vec3(1. , 0. , 0.);
            s.m_radius = 0.5f;
            s.build_arrays();
            s.material.type = Material_Mirror;
            s.material.diffuse_material = Vec3( 0.,1.,0. );
            s.material.specular_material = Vec3( 0.2,0.2,0.2 );
            s.material.shininess = 20;
        }
    }

    void setup_single_square() {
        meshes.clear();
        spheres.clear();
        squares.clear();
        lights.clear();

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

    void setup_cornell_box(){
        meshes.clear();
        spheres.clear();
        squares.clear();
        lights.clear();

        {
            lights.resize( lights.size() + 1 );
            Light & light = lights[lights.size() - 1];
            // base settings : 0.0    1.5      0.0
            light.pos = Vec3( 0.0, 1.5, 0.0 );
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
            s.translate(Vec3(0., 0., -2.));
            s.build_arrays();
            s.material.diffuse_material = Vec3( 1.,1.,1. );
            s.material.specular_material = Vec3( 1.,1.,1. );
            s.material.shininess = 16;
        }

        { //Left Wall

            squares.resize( squares.size() + 1 );
            Square & s = squares[squares.size() - 1];
            s.setQuad(Vec3(-1., -1., 0.), Vec3(1., 0, 0.), Vec3(0., 1, 0.), 2., 2.);
            s.scale(Vec3(2., 2., 1.));
            s.translate(Vec3(0., 0., -2.));
            s.rotate_y(90);
            s.build_arrays();
            s.material.diffuse_material = Vec3( 1.,0.,0. );
            s.material.specular_material = Vec3( 1.,0.,0. );
            s.material.shininess = 16;
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

        { //Floor
            squares.resize( squares.size() + 1 );
            Square & s = squares[squares.size() - 1];
            s.setQuad(Vec3(-1., -1., 0.), Vec3(1., 0, 0.), Vec3(0., 1, 0.), 2., 2.);
            s.translate(Vec3(0., 0., -2.));
            s.scale(Vec3(2., 2., 1.));
            s.rotate_x(-90);
            s.build_arrays();
            s.material.diffuse_material = Vec3( 1.0,1.0,1.0 );
            s.material.specular_material = Vec3( 1.0,1.0,1.0 );
            s.material.shininess = 16;
            s.material.texture_type = Texture_Checkerboard;
            s.material.checkerboard_color1 = Vec3(1.);
            s.material.checkerboard_color2 = Vec3(0.);
            s.material.checkerboard_scale = 8.;
        }

        { //Ceiling
            squares.resize( squares.size() + 1 );
            Square & s = squares[squares.size() - 1];
            s.setQuad(Vec3(-1., -1., 0.), Vec3(1., 0, 0.), Vec3(0., 1, 0.), 2., 2.);
            s.translate(Vec3(0., 0., -2.));
            s.scale(Vec3(2., 2., 1.));
            s.rotate_x(90);
            s.build_arrays();
            s.material.diffuse_material = Vec3( 1.0,1.0,1.0 );
            s.material.specular_material = Vec3( 1.0,1.0,1.0 );
            s.material.shininess = 16;
        }
       
       
  
        { //Front Wall
            squares.resize( squares.size() + 1 );
            Square & s = squares[squares.size() - 1];
            s.setQuad(Vec3(-1., -1., 0.), Vec3(1., 0, 0.), Vec3(0., 1, 0.), 2., 2.);
            s.translate(Vec3(0., 0., -2.));
            s.scale(Vec3(2., 2., 1.));
            s.rotate_y(180);
            s.build_arrays();
            s.material.diffuse_material = Vec3( 1.0,1.0,1.0 );
            s.material.specular_material = Vec3( 1.0,1.0,1.0 );
            s.material.shininess = 16;
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

    }

    void setup_rt_in_a_weekend() {
        meshes.clear();
        spheres.clear();
        squares.clear();
        lights.clear();

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
            s.material.load_texture("img/sphereTextures/s2.ppm");
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
            s.material.checkerboard_scale = 100.;
        }
    }

    void setup_mesh() {
        meshes.clear();
        spheres.clear();
        squares.clear();
        lights.clear();

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
            m.loadOFF("mesh/blob-closed.off", false);
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
    }

    void setup_random_spheres() {
        meshes.clear();
        spheres.clear();
        squares.clear();
        lights.clear();
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
        meshes.clear();
        spheres.clear();
        squares.clear();
        lights.clear();
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
        meshes.clear();
        spheres.clear();
        squares.clear();
        lights.clear();
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
            s.material.checkerboard_scale = 100.;
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
            m.loadOFF("mesh/flamingo_lowpoly_colored.off", true);
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
    }
};



#endif
