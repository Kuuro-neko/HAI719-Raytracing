#ifndef EQ_H
#define EQ_H

#include "Vec3.h"
#include "math.h"

float dot_product(Vec3 u, Vec3 v) {
    return u[0] * v[0] + u[1] * v[1] + u[2] * v[2];
}
/*
Vec3 cross_product(Vec3 u, Vec v) {

}*/

float norme(Vec3 u) {
    float res = 0.;
    for (int i = 0; i < 3; i++) {
        res += pow(u[i],2) ;
    }
    return sqrt(res);
}

float solve_eq_2nd_degre(int a, int b , int c) {
    float delta = pow(b, 2) - 4*a*c;
    if (delta < 0) {} 
    return 0.;
}

#endif