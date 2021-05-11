﻿// A practical implementation of the ray tracing algorithm.

#include "geometry.h"
#include "SDL.h" 
#include "Ray.h"
#include <fstream>
#include <chrono>

#define M_PI 3.14159265359

SDL_Window* window;
SDL_Renderer* renderer;
SDL_Surface* screen;

void init() {
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* window = SDL_CreateWindow(
        "Software Ray Tracer",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        640,
        480,
        0
    );

    screen = SDL_GetWindowSurface(window);

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
}

void putpixel(SDL_Surface* surface, int x, int y, Uint32 pixel)
{
    int bpp = surface->format->BytesPerPixel;
    /* Here p is the address to the pixel we want to set */
    Uint8* p = (Uint8*)surface->pixels + y * surface->pitch + x * bpp;

    switch (bpp) {
    case 1:
        *p = pixel;
        break;

    case 2:
        *(Uint16*)p = pixel;
        break;

    case 3:
        if (SDL_BYTEORDER == SDL_BIG_ENDIAN) {
            p[0] = (pixel >> 16) & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = pixel & 0xff;
        }
        else {
            p[0] = pixel & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = (pixel >> 16) & 0xff;
        }
        break;

    case 4:
        *(Uint32*)p = pixel;
        break;
    }
}
////The Code her is now replaced with the class below
//struct Ray {
//    Point3f o;
//    Vec3f d;
//    Ray(const Point3f& o, const Vec3f& d) : o(o), d(d) {}
//};


struct Sphere {
    Point3f c;
    double r;
    Sphere(const Point3f& c, double r) : c(c), r(r) {}
    Vec3f getNormal(const Vec3f& pi) const { return (pi - c) / r; }

    // Solve t^2*d.d+2*t*(o-p).d+(o-p).(o-p)-R^2=0​
    bool intersect(const Ray& ray, double& t) const {
        const Point3f o = ray.o;
        const Vec3f d = ray.d;
        const Vec3f oc = o - c;
        const double b = 2 * oc.dotProduct(d);
        const double c = oc.dotProduct(oc) - r * r;
        double disc = b * b - 4 * c; // a=1 as ray is normalised​
        if (disc < 1e-4) return false; // ray misses sphere​
        disc = sqrt(disc);
        const double t0 = -b - disc;
        const double t1 = -b + disc;
        t = (t0 < t1) ? t0 : t1; // two intersections on sphere, set t to shortest​
        return true;
    }
};

// method to ensure colours don’t go out of 8 bit range in RGB​
void clamp255(Vec3f& col) {
    col.x = (col.x > 255) ? 255 : (col.x < 0) ? 0 : col.x;
    col.y = (col.y > 255) ? 255 : (col.y < 0) ? 0 : col.y;
    col.z = (col.z > 255) ? 255 : (col.z < 0) ? 0 : col.z;
}

double hit_sphere(const Point3f& centre, double radius, const Ray& r) {
    Vec3f oc = r.origin() - centre;
    auto a = r.direction().dotProduct(r.direction());
    auto b = 2.0 * oc.dotProduct(r.direction());
    auto c = oc.dotProduct(oc) - radius * radius;
    auto discriminant = b * b - 4 * a * c;
    if (discriminant < 0) {
        return -1.0;
    }
    else {
        return (-b - sqrt(discriminant)) / (2.0 * a);
    }
}

Colour ray_colour(const Ray& r) {
    auto t = hit_sphere(Point3f(0, 0, -1), 0.5, r);
    if (t > 0.0) {
        Vec3f N = (r.at(t) - Vec3f(0, 0, -1)).normalize(); 
        return 0.5 * Colour(N.x + 1, N.y + 1, N.z + 1) * 255;
    }
    Vec3f unit_direction = r.direction().normalize();
    t = 0.5 * (unit_direction.y + 1);
    return (1.0 - t) * Colour(1.0, 1.0, 1.0) + t * Colour(0.5, 0.7, 1.0) * 255;
}

int main(int argc, char **argv)
{
    // initialise SDL2
    init();

    //Image(should be in main.ccp)
    const auto aspect_ratio = 16.0 / 9.0;
    const int image_width = screen->w;
    const int image_height = static_cast<int>(image_width / aspect_ratio);

    //camera (should be in main.ccp)
    auto viewport_height = 2.0;
    auto viewport_width = aspect_ratio * viewport_height;
    auto focal_length = 1.0;
    auto origin = Point3f(0, 0, 0);
    auto horizontal = Vec3f(viewport_width, 0, 0);
    auto vertical = Vec3f(0, viewport_height, 0);
    auto lower_left_corner = origin - horizontal / 2 - vertical / 2 - Vec3f(0, 0, focal_length);


    const Colour white(255, 255, 255);
    const Colour black(0, 0, 0);
    const Colour red(255, 0, 0);

    const Sphere sphere(Vec3f(screen->w * 0.5, screen->h * 0.5, 50), 50);
    const Sphere light(Vec3f(0, 0, 50), 1);

    double t;
    Colour pix_col(black);

    SDL_Event e;
    bool running = true;
    while (running) {

        auto t_start = std::chrono::high_resolution_clock::now();

        // clear back buffer, pixel data on surface and depth buffer (as movement)
        SDL_FillRect(screen, nullptr, SDL_MapRGB(screen->format, 0, 0, 0));
        SDL_RenderClear(renderer);

        ////Replaced with for loop below
        //for (int y = 0; y < screen->h; ++y) {
        //    for (int x = 0; x < screen->w; ++x) {
        //        pix_col = black;
        //        const Ray ray(Point3f(x, y, 0), Vec3f(0, 0, 1));
        //        if (sphere.intersect(ray, t)) {
        //            const Vec3f pi = ray.o + ray.d * t;
        //            Vec3f L = light.c - pi;
        //            Vec3f N = sphere.getNormal(pi);
        //            Vec3f Ln = L.normalize();
        //            double dt = Ln.dotProduct(N.normalize());
        //            pix_col = (red + white * dt) * 0.5;
        //            clamp255(pix_col);
        //        }
        //        Uint32 colour = SDL_MapRGB(screen->format, pix_col.x, pix_col.y, pix_col.z);
        //        putpixel(screen, x, y, colour);
        //    }
        //}

        for (int y = screen->h - 1; y >= 0; y--) {//starts from the top left
            std::cerr << "\rScanLines reamining: " << y << std::flush;
            for (int x = 0; x < screen->w; ++x) {
                auto u = double(x) / (image_width - 1);
                auto v = double(y) / (image_height - 1);
                Ray ray(origin, lower_left_corner + u * horizontal + v * vertical - origin);
                Colour pix_col = ray_colour(ray);
                Uint32 colour = SDL_MapRGB(screen->format, pix_col.x, pix_col.y, pix_col.z);
                putpixel(screen, x, y, colour);
            }
        }

        auto t_end = std::chrono::high_resolution_clock::now();
        auto passedTime = std::chrono::duration<double, std::milli>(t_end - t_start).count();
        std::cerr << "Frame render time:  " << passedTime << " ms" << std::endl;

        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, screen);
        if (texture == NULL) {
            fprintf(stderr, "CreateTextureFromSurface failed: %s\n", SDL_GetError());
            exit(1);
        }
        SDL_FreeSurface(screen);

        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);

        SDL_DestroyTexture(texture);

        if (SDL_PollEvent(&e))
        {
            switch (e.type) {
            case SDL_KEYDOWN:
                switch (e.key.keysym.sym) {
                case SDLK_ESCAPE:
                    running = false;
                    break;
                }
                break;
            }
        }
    }

    return 0;
}