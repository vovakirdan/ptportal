#include "defs.h"

// -1 right, 0 on, 1 left
#define point_side(_p, _a, _b) ({ \
    __typeof__(_p) __p = (_p), __a = (_a), __b = (_b); \
    -(((__p.x - __a.x) * (__b.y - __a.y)) \
    - ((__p.y - __a.y) * (__b.x - __a.x))); \
})

// rotate vector v by angle a
static inline v2 rotate(v2 v, f32 a) {
    return (v2) {
        (v.x * cos(a)) - (v.y * sin(a)),
        (v.x * sin(a)) + (v.y * cos(a)),
    };
}

// compute intersection of two line segments, return (NAN, NAN) if no intersection
static inline v2 intersect_segs(v2 a0, v2 a1, v2 b0, v2 b1) {
    // calculate determinant
    const f32 d = ((a0.x - a1.x) * (b0.y - b1.y)) - ((a0.y - a1.y) * (b0.x - b1.x));
    
    if (fabsf(d) < EPSILON) {return (v2) {NAN, NAN};}
    
}