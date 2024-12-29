#ifndef RAY_H
#define RAY_H
#include "Line.h"
class Ray : public Line {
public:
    float time;
    Ray() : Line() {}
    Ray( Vec3 const & o , Vec3 const & d, float time ) : Line(o,d), time(time) {}
};
#endif
