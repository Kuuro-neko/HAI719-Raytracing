#ifndef Sphere_H
#define Sphere_H
#include "Vec3.h"
#include <vector>
#include "Mesh.h"
#include <cmath>
#include "math.h"
#include "Functions.h"
#include "AABB.h"

struct RaySphereIntersection{
    bool intersectionExists;
    float t;
    float theta,phi;
    Vec3 intersection;
    Vec3 secondintersection;
    Vec3 normal;

    RaySphereIntersection() : intersectionExists(false) , t(FLT_MAX) {}
};

static
Vec3 SphericalCoordinatesToEuclidean( Vec3 ThetaPhiR ) {
    return ThetaPhiR[2] * Vec3( cos(ThetaPhiR[0]) * cos(ThetaPhiR[1]) , sin(ThetaPhiR[0]) * cos(ThetaPhiR[1]) , sin(ThetaPhiR[1]) );
}
static
Vec3 SphericalCoordinatesToEuclidean( float theta , float phi ) {
    return Vec3( cos(theta) * cos(phi) , sin(theta) * cos(phi) , sin(phi) );
}

static
Vec3 EuclideanCoordinatesToSpherical( Vec3 xyz ) {
    float R = xyz.length();
    float phi = asin( xyz[2] / R );
    float theta = atan2( xyz[1] , xyz[0] );
    return Vec3( theta , phi , R );
}



class Sphere : public Mesh {
public:
    Vec3 m_center;
    float m_radius;

    Sphere() : Mesh() {}
    Sphere(Vec3 c , float r) : Mesh() , m_center(c) , m_radius(r) {}

    void build_arrays(){
        unsigned int nTheta = 20 , nPhi = 20;
        positions_array.resize(3 * nTheta * nPhi );
        normalsArray.resize(3 * nTheta * nPhi );
        uvs_array.resize(2 * nTheta * nPhi );
        for( unsigned int thetaIt = 0 ; thetaIt < nTheta ; ++thetaIt ) {
            float u = (float)(thetaIt) / (float)(nTheta-1);
            float theta = u * 2 * M_PI;
            for( unsigned int phiIt = 0 ; phiIt < nPhi ; ++phiIt ) {
                unsigned int vertexIndex = thetaIt + phiIt * nTheta;
                float v = (float)(phiIt) / (float)(nPhi-1);
                float phi = - M_PI/2.0 + v * M_PI;
                Vec3 xyz = SphericalCoordinatesToEuclidean( theta , phi );
                positions_array[ 3 * vertexIndex + 0 ] = m_center[0] + m_radius * xyz[0];
                positions_array[ 3 * vertexIndex + 1 ] = m_center[1] + m_radius * xyz[1];
                positions_array[ 3 * vertexIndex + 2 ] = m_center[2] + m_radius * xyz[2];
                normalsArray[ 3 * vertexIndex + 0 ] = xyz[0];
                normalsArray[ 3 * vertexIndex + 1 ] = xyz[1];
                normalsArray[ 3 * vertexIndex + 2 ] = xyz[2];
                uvs_array[ 2 * vertexIndex + 0 ] = u;
                uvs_array[ 2 * vertexIndex + 1 ] = v;
            }
        }
        triangles_array.clear();
        for( unsigned int thetaIt = 0 ; thetaIt < nTheta - 1 ; ++thetaIt ) {
            for( unsigned int phiIt = 0 ; phiIt < nPhi - 1 ; ++phiIt ) {
                unsigned int vertexuv = thetaIt + phiIt * nTheta;
                unsigned int vertexUv = thetaIt + 1 + phiIt * nTheta;
                unsigned int vertexuV = thetaIt + (phiIt+1) * nTheta;
                unsigned int vertexUV = thetaIt + 1 + (phiIt+1) * nTheta;
                triangles_array.push_back( vertexuv );
                triangles_array.push_back( vertexUv );
                triangles_array.push_back( vertexUV );
                triangles_array.push_back( vertexuv );
                triangles_array.push_back( vertexUV );
                triangles_array.push_back( vertexuV );
            }
        }
        computeAABBFromPosArray();
    }

    // TO DO add , float time = 0.f to each intersect function (square and sphere and mesh) for motion blur
    RaySphereIntersection intersect(const Ray &ray) const {
        RaySphereIntersection intersection;
        //TODO calcul l'intersection rayon sphere
        Vec3 timed_center = m_center + (ray.time) * material.motion_blur_translation;
        Vec3 o = ray.origin();
        Vec3 d = ray.direction();
        float t, t1;
        float r = m_radius;
        float delta, a, b, c;
        a = Vec3::dot(d,d);
        b = 2.*Vec3::dot(d, (o-timed_center));
        c = Vec3::dot((o-timed_center),(o-timed_center)) - r*r;
        delta = b*b - 4*a*c;

        if (delta < 0) {
            intersection.intersectionExists = false;
            intersection.t = FLT_MAX;
            return intersection;
        }

        float sqrtDelta = sqrt(delta);
        t = (-b - sqrtDelta) / (2 * a);
        t1 = (-b + sqrtDelta) / (2 * a);

        if (t1 > EPSILON && t1 < t ) {
            t = t1;
        }

        if (t < -EPSILON) {
            intersection.intersectionExists = false;
            intersection.t = FLT_MAX;
            return intersection;
        }
        intersection.intersection = o + t*d;
        intersection.normal = (intersection.intersection - timed_center);
        intersection.normal.normalize();
        intersection.intersectionExists = true;
        intersection.t = t;
        intersection.theta = acos(intersection.normal[1]*-1.);
        intersection.phi = atan2(intersection.normal[2]*-1., intersection.normal[0]) + M_PI;
        return intersection;
    }
};
#endif
