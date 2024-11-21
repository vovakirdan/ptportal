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
#define sign(a) ({                                       \
        __typeof__(a) _a = (a);                          \
        (__typeof__(a))(_a < 0 ? -1 : (_a > 0 ? 1 : 0)); \
    })
#define MAP_SIZE 8

static u8 MAPDATA[MAP_SIZE * MAP_SIZE] = {
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 3, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 2, 0, 4, 4, 0, 1,
    1, 0, 0, 0, 4, 0, 0, 1,
    1, 0, 3, 0, 0, 0, 0, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
};

struct {
    SDL_Window *window;
    SDL_Texture *texture;
    SDL_Renderer *renderer;
    u32 pixels[SCREEN_WIDTH * SCREEN_HEIGHT];
    bool quit;

    v2 pos, dir, plane;
} state;

static void verline(int x, int y0, int y1, u32 color) {
    for (int y = y0; y <= y1; y++) {
        state.pixels[(y * SCREEN_WIDTH) + x] = color;
    }
}

static void render() {
    for (int x = 0; x < SCREEN_WIDTH; x++) {
        // x coordinate in space from [-1, 1]
        const f32 xcam = (2 * (x / (f32) (SCREEN_WIDTH))) - 1;

        // ray direction through this column
        const v2 dir = {
            state.dir.x + state.plane.x * xcam,
            state.dir.y + state.plane.y * xcam
        };

        v2 pos = state.pos;
        v2i ipos = { (int) pos.x, (int) pos.y };

        // distance ray must travel from one x/y side to the next
        const v2 deltadist = {
            fabsf(dir.x) < 1e-20 ? 1e30 : fabsf(1.0f / dir.x),
            fabsf(dir.y) < 1e-20 ? 1e30 : fabsf(1.0f / dir.y),
        };

        // distance from start position to first x/y side
        v2 sidedist = {
            deltadist.x * (dir.x < 0 ? (pos.x - ipos.x) : (ipos.x + 1 - pos.x)),
            deltadist.y * (dir.y < 0 ? (pos.y - ipos.y) : (ipos.y + 1 - pos.y)),
        };

        // integer step direction for x/y, calculated from overall diff
        const v2i step = { (int) sign(dir.x), (int) sign(dir.y) };

        // DDA hit
        struct { int val, side; v2 pos; } hit = { 0, 0, { 0.0f, 0.0f } };

        while (!hit.val) {
            if (sidedist.x < sidedist.y) {
                sidedist.x += deltadist.x;
                ipos.x += step.x;
                hit.side = 0;
            } else {
                sidedist.y += deltadist.y;
                ipos.y += step.y;
                hit.side = 1;
            }

            ASSERT(
                ipos.x >= 0
                && ipos.x < MAP_SIZE
                && ipos.y >= 0
                && ipos.y < MAP_SIZE,
                "DDA out of bounds");

            hit.val = MAPDATA[ipos.y * MAP_SIZE + ipos.x];
        }

        u32 color;
        switch (hit.val) {
        case 1: color = 0xFF0000FF; break;
        case 2: color = 0xFF00FF00; break;
        case 3: color = 0xFFFF0000; break;
        case 4: color = 0xFFFF00FF; break;
        }

        // darken colors on y-sides
        if (hit.side == 1) {
            const u32
                br = ((color & 0xFF00FF) * 0xC0) >> 8,
                g  = ((color & 0x00FF00) * 0xC0) >> 8;

            color = 0xFF000000 | (br & 0xFF00FF) | (g & 0x00FF00);
        }

        hit.pos = (v2) { pos.x + sidedist.x, pos.y + sidedist.y };

        // distance to hit
        const f32 dperp =
            hit.side == 0 ?
                (sidedist.x - deltadist.x)
                : (sidedist.y - deltadist.y);

        // perform perspective division, calculate line height relative to
        // screen center
        const int
            h = (int) (SCREEN_HEIGHT / dperp),
            y0 = max((SCREEN_HEIGHT / 2) - (h / 2), 0),
            y1 = min((SCREEN_HEIGHT / 2) + (h / 2), SCREEN_HEIGHT - 1);

        verline(x, 0, y0, 0xFF202020);
        verline(x, y0, y1, color);
        verline(x, y1, SCREEN_HEIGHT - 1, 0xFF505050);
    }
}

static void rotate(f32 rot) {
    const v2 d = state.dir, p = state.plane;
    state.dir.x = d.x * cos(rot) - d.y * sin(rot);
    state.dir.y = d.x * sin(rot) + d.y * cos(rot);
    state.plane.x = p.x * cos(rot) - p.y * sin(rot);
    state.plane.y = p.x * sin(rot) + p.y * cos(rot);
}

int main(int argc, char *argv[]) {
    ASSERT(
        !SDL_Init(SDL_INIT_VIDEO),
        "SDL failed to initialize: %s\n",
        SDL_GetError());

    state.window =
        SDL_CreateWindow(
            "DEMO",
            SDL_WINDOWPOS_CENTERED_DISPLAY(0),
            SDL_WINDOWPOS_CENTERED_DISPLAY(0),
            1280,
            720,
            SDL_WINDOW_ALLOW_HIGHDPI);
    ASSERT(
        state.window,
        "failed to create SDL window: %s\n", SDL_GetError());

    state.renderer =
        SDL_CreateRenderer(state.window, -1, SDL_RENDERER_PRESENTVSYNC);
    ASSERT(
        state.renderer,
        "failed to create SDL renderer: %s\n", SDL_GetError());

    state.texture =
        SDL_CreateTexture(
            state.renderer,
            SDL_PIXELFORMAT_ABGR8888,
            SDL_TEXTUREACCESS_STREAMING,
            SCREEN_WIDTH,
            SCREEN_HEIGHT);
    ASSERT(
        state.texture,
        "failed to create SDL texture: %s\n", SDL_GetError());

    state.pos = (v2) { 2, 2 };
    state.dir = normalize(((v2) { -1.0f, 0.1f }));
    state.plane = (v2) { 0.0f, 0.66f };

    const int FPS = 60;
    const int frameDelay = 1000 / FPS;

    u32 frameStart;
    int frameTime;

    while (!state.quit) {
        frameStart = SDL_GetTicks();

        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            switch (ev.type) {
                case SDL_QUIT:
                    state.quit = true;
                    break;
            }
        }

        const f32
            rotspeed = 3.0f * 0.016f,
            movespeed = 3.0f * 0.016f;

        const u8 *keystate = SDL_GetKeyboardState(NULL);
        if (keystate[SDL_SCANCODE_LEFT]) {
            rotate(+rotspeed);
        }

        if (keystate[SDL_SCANCODE_RIGHT]) {
            rotate(-rotspeed);
        }

        if (keystate[SDL_SCANCODE_UP]) {
            state.pos.x += state.dir.x * movespeed;
            state.pos.y += state.dir.y * movespeed;
        }

        if (keystate[SDL_SCANCODE_DOWN]) {
            state.pos.x -= state.dir.x * movespeed;
            state.pos.y -= state.dir.y * movespeed;
        }

        memset(state.pixels, 0, sizeof(state.pixels));
        render();

        SDL_UpdateTexture(state.texture, NULL, state.pixels, SCREEN_WIDTH * 4);
        SDL_RenderCopyEx(
            state.renderer,
            state.texture,
            NULL,
            NULL,
            0.0,
            NULL,
            SDL_FLIP_VERTICAL);
        SDL_RenderPresent(state.renderer);

        frameTime = SDL_GetTicks() - frameStart;
        if (frameDelay > frameTime) {
            SDL_Delay(frameDelay - frameTime);
        }
    }

    SDL_DestroyTexture(state.texture);
    SDL_DestroyRenderer(state.renderer);
    SDL_DestroyWindow(state.window);
    return 0;
}