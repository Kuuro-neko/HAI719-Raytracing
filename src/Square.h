#ifndef SQUARE_H
#define SQUARE_H
#include "Vec3.h"
#include <vector>
#include "Mesh.h"
#include <cmath>
#include "Functions.h"

struct RaySquareIntersection{
    bool intersectionExists;
    float t;
    float u,v;
    Vec3 intersection;
    Vec3 normal;
};


class Square : public Mesh {
public:
    Vec3 m_normal;
    Vec3 m_bottom_left;
    Vec3 m_right_vector;
    Vec3 m_up_vector;

    Square() : Mesh() {}
    Square(Vec3 const & bottomLeft , Vec3 const & rightVector , Vec3 const & upVector , float width=1. , float height=1. ,
           float uMin = 0.f , float uMax = 1.f , float vMin = 0.f , float vMax = 1.f) : Mesh() {
        setQuad(bottomLeft, rightVector, upVector, width, height, uMin, uMax, vMin, vMax);
        computeAABB();
    }

    void setQuad( Vec3 const & bottomLeft , Vec3 const & rightVector , Vec3 const & upVector , float width=1. , float height=1. ,
                  float uMin = 0.f , float uMax = 1.f , float vMin = 0.f , float vMax = 1.f) {
        m_right_vector = rightVector;
        m_up_vector = upVector;
        m_normal = Vec3::cross(rightVector , upVector);
        m_bottom_left = bottomLeft;

        m_normal.normalize();
        m_right_vector.normalize();
        m_up_vector.normalize();

        m_right_vector = m_right_vector*width;
        m_up_vector = m_up_vector*height;

        vertices.clear();
        vertices.resize(4);
        vertices[0].position = bottomLeft;                                      vertices[0].u = uMin; vertices[0].v = vMin;
        vertices[1].position = bottomLeft + m_right_vector;                     vertices[1].u = uMax; vertices[1].v = vMin;
        vertices[2].position = bottomLeft + m_right_vector + m_up_vector;       vertices[2].u = uMax; vertices[2].v = vMax;
        vertices[3].position = bottomLeft + m_up_vector;                        vertices[3].u = uMin; vertices[3].v = vMax;
        vertices[0].normal = vertices[1].normal = vertices[2].normal = vertices[3].normal = m_normal;
        triangles.clear();
        triangles.resize(2);
        triangles[0][0] = 0;
        triangles[0][1] = 1;
        triangles[0][2] = 2;
        triangles[1][0] = 0;
        triangles[1][1] = 2;
        triangles[1][2] = 3;

    }

    RaySquareIntersection intersect(const Ray &ray) const {
        RaySquareIntersection intersection;
        if(!aabb.intersects(ray)) return intersection;

        Vec3 m_bottom_left = vertices[0].position;
        Vec3 m_right_vector = vertices[1].position - vertices[0].position;
        Vec3 m_up_vector = vertices[3].position - vertices[0].position;
        Vec3 m_normal = Vec3::cross(m_right_vector, m_up_vector);
        m_normal.normalize();

        //TODO calculer l'intersection rayon quad
        float dotRN = Vec3::dot(ray.direction(), m_normal);
        // Rayon parallèle
        if (dotRN == 0) {
            intersection.intersectionExists = false;
            intersection.t = FLT_MAX;
            return intersection;
        }

        if (dotRN > 0) {
            intersection.intersectionExists = false;
            intersection.t = FLT_MAX;
            return intersection;
        }
        
        float D = Vec3::dot(m_bottom_left, m_normal);
        float t = (D - Vec3::dot(ray.origin(), m_normal))/(dotRN);

        // Intersection derrière la caméra
        if (t < -EPSILON) {
            intersection.intersectionExists = false;
            intersection.t = FLT_MAX;
            return intersection;
        }

        if (t >= EPSILON) {
        
            Vec3 p = ray.origin() + t*ray.direction();
            Vec3 q = p - m_bottom_left;

            // proj sur bottom left to right
            float proj1 = Vec3::dot(q, m_right_vector) / m_right_vector.length();


            // proj sur bottom left to up
            float proj2 = Vec3::dot(q, m_up_vector) / m_up_vector.length();

            if ((proj1 <= m_right_vector.length() && proj1 >= 0) && (proj2 <= m_up_vector.length() && proj2 >= 0)) {
                intersection.intersectionExists = true;
                intersection.t = t;
                intersection.u = proj1 / m_right_vector.length();
                intersection.v = proj2 / m_up_vector.length();
                intersection.intersection = p;
                intersection.normal = m_normal;
                return intersection;
            }

        }
        intersection.intersectionExists = false;
        intersection.t = FLT_MAX;
        return intersection;
    }
};
#endif // SQUARE_H
