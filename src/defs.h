#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <ctype.h>
#include <math.h>
// #include <SDL.h>
#include <SDL2/SDL.h>

#define ASSERT(_e, ...) if (!(_e)) { fprintf(stderr, __VA_ARGS__); exit(1);}

typedef float   f32;
typedef double  f64;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef size_t usize;
typedef ssize_t isize;

#define PI 3.1415926535897932384626433832795028841971693993751058209749445923078164062862089986280348253421170679821480865132823066470938446095505822317253594081
#define TAU 6.28318530717958647692528676655900576839433879875021164194988918402352993635854312322296291561932865608555741189163895218679342768281325220201609
#define PI_2 (PI / 2.f)
#define PI_4 (PI / 4.f)
#define EPSILON 0.000001f

#define DEG_TO_RAD(_d) ((_d) * (PI / 180.f))
#define RAD_TO_DEG(_r) ((_r) * (180.f / PI))

#define SCREEN_WIDTH 384
#define SCREEN_HEIGHT 216

#define EYE_Z 1.65f
#define HFOV DEG_TO_RAD(90.f)
#define VFOV 0.5f

#define ZNEAR 0.0001f
#define ZFAR 128.0f

// two dimensional vector
typedef struct v2_s {f32 x, y;} v2;
// two dimensional integer vector
typedef struct v2i_s {i32 x, y;} v2i;

// convert float vector to integer vector
#define v2_to_v2i(_v) ({__typeof__(_v) __v = (_v); (v2i) {__v.x, __v.y};})
// convert integer vector to float vector
#define v2i_to_v2(_v) ({__typeof__(_v) __v = (_v); (v2) {__v.x, __v.y};})

#define dot(_v0, _v1) ({__typeof__(_v0) __v0 = (_v0), __v1 = (_v1); (__v0.x * __v1.x) + (__v0.y * __v1.y);})
#define length(_vl) ({__typeof__(_vl) __vl = (_vl); sqrtf(dot(__vl, __vl));})
#define normalize(_vn) ({ __typeof__(_vn) __vn = (_vn); const f32 l = length(__vn); (__typeof__(_vn)) { __vn.x / l, __vn.y / l }; })
#define min(_a, _b) ({ __typeof__(_a) __a = (_a), __b = (_b); __a < __b ? __a : __b; })
#define max(_a, _b) ({ __typeof__(_a) __a = (_a), __b = (_b); __a > __b ? __a : __b; })
#define clamp(_x, _mi, _ma) (min(max(_x, _mi), _ma))
#define ifnan(_x, _alt) ({ __typeof__(_x) __x = (_x); isnan(__x) ? (_alt) : __x; })

#define MAP_SIZE 8