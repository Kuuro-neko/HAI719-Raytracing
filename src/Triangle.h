#ifndef TRIANGLE_H
#define TRIANGLE_H
#include "Vec3.h"
#include "Ray.h"
#include "Plane.h"
#include <cfloat>
#include "AABB.h"

struct RayTriangleIntersection{
    bool intersectionExists;
    float t;
    float w0,w1,w2;
    unsigned int tIndex;
    Vec3 intersection;
    Vec3 normal;
};

class Triangle {
private:
    Vec3 m_c[3] , m_normal;
    float area;
public:
    Triangle() {}
    Triangle( Vec3 const & c0 , Vec3 const & c1 , Vec3 const & c2 ) {
        m_c[0] = c0;
        m_c[1] = c1;
        m_c[2] = c2;
        updateAreaAndNormal();
    }
    void updateAreaAndNormal() {
        Vec3 nNotNormalized = Vec3::cross( m_c[1] - m_c[0] , m_c[2] - m_c[0] );
        float norm = nNotNormalized.length();
        m_normal = nNotNormalized / norm;
        area = norm / 2.f;
    }
    void setC0( Vec3 const & c0 ) { m_c[0] = c0; } // remember to update the area and normal afterwards!
    void setC1( Vec3 const & c1 ) { m_c[1] = c1; } // remember to update the area and normal afterwards!
    void setC2( Vec3 const & c2 ) { m_c[2] = c2; } // remember to update the area and normal afterwards!
    Vec3 const & normal() const { return m_normal; }
    Vec3 projectOnSupportPlane( Vec3 const & p ) const {
        Vec3 result;
        //TODO completer
        return result;
    }
    float squareDistanceToSupportPlane( Vec3 const & p ) const {
        float result;
        //TODO completer
        return result;
    }
    float distanceToSupportPlane( Vec3 const & p ) const { return sqrt( squareDistanceToSupportPlane(p) ); }
    bool isParallelTo( Line const & L ) const {
        return Vec3::dot( L.direction() , m_normal ) == 0;
    }
    Vec3 getIntersectionPointWithSupportPlane( Line const & L ) const {
        // you should check first that the line is not parallel to the plane!
        Vec3 result;
        //TODO completer
        return result;
    }
    void computeBarycentricCoordinates( Vec3 const & p , float & u0 , float & u1 , float & u2 ) const {
        Vec3 v0 = m_c[1] - m_c[0];
        Vec3 v1 = m_c[2] - m_c[0];
        Vec3 v2 = p - m_c[0];
        float d00 = Vec3::dot( v0 , v0 );
        float d01 = Vec3::dot( v0 , v1 );
        float d11 = Vec3::dot( v1 , v1 );
        float d20 = Vec3::dot( v2 , v0 );
        float d21 = Vec3::dot( v2 , v1 );
        float denom = d00 * d11 - d01 * d01;
        u1 = ( d11 * d20 - d01 * d21 ) / denom;
        u2 = ( d00 * d21 - d01 * d20 ) / denom;
        u0 = 1 - u1 - u2;
    }

    RayTriangleIntersection getIntersection( Ray const & ray ) const {
        RayTriangleIntersection result;
        float dotRN = Vec3::dot( ray.direction() , m_normal );
        // 1) check that the ray is not parallel to the triangle:
        if (dotRN == 0) {
            result.intersectionExists = false;
            result.t = FLT_MAX;
            return result;
        }

        // Check if in front of the triangle
        if (dotRN > 0) {
            result.intersectionExists = false;
            result.t = FLT_MAX;
            return result;
        }

        // 2) check that the triangle is "in front of" the ray:
        float D = Vec3::dot( m_c[0] , m_normal );
        float t = ( D - Vec3::dot( ray.origin() , m_normal ) ) / dotRN;
        if ( t < 0 ) {
            result.intersectionExists = false;
            result.t = FLT_MAX;
            return result;
        }

        // 3) check that the intersection point is inside the triangle:
        // CONVENTION: compute u,v such that p = w0*c0 + w1*c1 + w2*c2, check that 0 <= w0,w1,w2 <= 1
        Vec3 p = ray.origin() + t * ray.direction();

        float u0,u1,u2;
        computeBarycentricCoordinates( p , u0 , u1 , u2 );
        

        // 4) Finally, if all conditions were met, then there is an intersection! :
        if ( u0 >= 0 && u0 <= 1 && u1 >= 0 && u1 <= 1 && u2 >= 0 && u2 <= 1 ) {
            result.intersectionExists = true;
            result.t = t;
            result.w0 = u0;
            result.w1 = u1;
            result.w2 = u2;
            result.intersection = p;
            result.normal = m_normal;
            return result;
        } else {
            result.intersectionExists = false;
            result.t = FLT_MAX;
            return result;
        }
    }

    AABB getAABB() {
        Vec3 p0(FLT_MAX);
        Vec3 p1(-FLT_MAX);
        for (int i = 0; i < 3; i++) {
            p0[i] = std::min(p0[i], m_c[0][i]);
            p0[i] = std::min(p0[i], m_c[1][i]);
            p0[i] = std::min(p0[i], m_c[2][i]);
            p1[i] = std::max(p1[i], m_c[0][i]);
            p1[i] = std::max(p1[i], m_c[1][i]);
            p1[i] = std::max(p1[i], m_c[2][i]);
        }
        return AABB(p0, p1);
    }
};
#endif
