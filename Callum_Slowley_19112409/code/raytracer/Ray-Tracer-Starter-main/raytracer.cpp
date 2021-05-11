// A practical implementation of the ray tracing algorithm.

#include "geometry.h"
#include "SDL.h" 
#include "Ray.h"
#include "sphere.h"
#include "common.h"
#include "Camera.h"
#include "hittable.h"
#include "hittable_list.h"
#include "material.h"
#include "Multithreading.h"
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


//struct Sphere {
//    Point3f c;
//    double r;
//    Sphere(const Point3f& c, double r) : c(c), r(r) {}
//    Vec3f getNormal(const Vec3f& pi) const { return (pi - c) / r; }
//
//    // Solve t^2*d.d+2*t*(o-p).d+(o-p).(o-p)-R^2=0​
//    bool intersect(const Ray& ray, double& t) const {
//        const Point3f o = ray.o;
//        const Vec3f d = ray.d;
//        const Vec3f oc = o - c;
//        const double b = 2 * oc.dotProduct(d);
//        const double c = oc.dotProduct(oc) - r * r;
//        double disc = b * b - 4 * c; // a=1 as ray is normalised​
//        if (disc < 1e-4) return false; // ray misses sphere​
//        disc = sqrt(disc);
//        const double t0 = -b - disc;
//        const double t1 = -b + disc;
//        t = (t0 < t1) ? t0 : t1; // two intersections on sphere, set t to shortest​
//        return true;
//    }
//};

// method to ensure colours don’t go out of 8 bit range in RGB​
void clamp255(Vec3f& col) {
    col.x = (col.x > 255) ? 255 : (col.x < 0) ? 0 : col.x;
    col.y = (col.y > 255) ? 255 : (col.y < 0) ? 0 : col.y;
    col.z = (col.z > 255) ? 255 : (col.z < 0) ? 0 : col.z;
}

//double hit_sphere(const Point3f& centre, double radius, const Ray& r) {
//    Vec3f oc = r.origin() - centre;
//    auto a = r.direction().dotProduct(r.direction());
//    auto b = 2.0 * oc.dotProduct(r.direction());
//    auto c = oc.dotProduct(oc) - radius * radius;
//    auto discriminant = b * b - 4 * a * c;
//    if (discriminant < 0) {
//        return -1.0;
//    }
//    else {
//        return (-b - sqrt(discriminant)) / (2.0 * a);
//    }
//}

Colour ray_colour(const Ray& r, const hittable& world, int depth) {
    hit_record rec;
    //if we have hit the depth limit no more light has been gathered
    if (depth <= 0)  return Colour(0, 0, 0); 
    if (world.hit(r,  0.001, infinity, rec)) {
        Ray scattered;
        Colour attenuation;
        if (rec.mat_ptr->scatter(r, rec, attenuation, scattered))
            return attenuation * ray_colour(scattered, world, depth - 1);
        return Colour(0, 0, 0);
    }
    Vec3f unit_direction = r.direction().normalize();
    auto t = 0.5 * (unit_direction.y + 1);
    return (1.0 - t) * Colour(1.0, 1.0, 1.0) + t * Colour(0.5, 0.7, 1.0) * 255;
}
void lineRender(SDL_Surface*screen, hittable_list world, int y, int spp, int max_depth, camera*cam) {
    const auto aspect_ratio = 16.0 / 9.0;
    const int image_width = screen->w;
    const int image_height = static_cast<int>(image_width / aspect_ratio);
    const Colour black(0, 0, 0);
    Colour pix_col(black);

        for (int x = 0; x < screen->w; ++x) {
            pix_col = black; //resets the colour per pixel to black
            for (int s = 0; s < spp; s++) {
                auto u = double(x + random_double()) / (image_width - 1);
                auto v = double(y + random_double()) / (image_height - 1);
                Ray ray = cam->get_ray(u, v);
                //colours for every sample
                pix_col = pix_col + ray_colour(ray, world, max_depth);
            }
            //scale spp and gamma correct
            pix_col /= 255.f * spp;
            pix_col.x = sqrt(pix_col.x);
            pix_col.y = sqrt(pix_col.y);
            pix_col.z = sqrt(pix_col.z);
            pix_col *= 255;
            Uint32 colour = SDL_MapRGB(screen->format, pix_col.x, pix_col.y, pix_col.z);
            putpixel(screen, x, y, colour);
        }
    }

int main(int argc, char **argv)
{
    // initialise SDL2
    init();

    //Image(should be in main.ccp)
    const auto aspect_ratio = 16.0 / 9.0;
    const int image_width = screen->w;
    const int image_height = static_cast<int>(image_width / aspect_ratio);
    const int max_depth = 50;

    //samples for testing
    const int spp = 4;
    const float scale = 1.0f / spp;

    //camera (should be in main.ccp)

    Point3f lookfrom(3, 3, 2);
    Point3f lookat(0, 0, -1);
    Vec3f vup(0, 1, 0);
    auto dist_to_focus = (lookfrom - lookat).length();
    auto aperture = 2.0;
    camera cam(lookfrom,lookat,vup,20,aspect_ratio,aperture,dist_to_focus);

    //world
    hittable_list world;

    //materials 
    auto material_ground = make_shared<lambertian>(Colour(0.8, 0.8, 0.0));
    auto material_center = make_shared<lambertian>(Colour(0.1, 0.2, 0.5));
    auto material_left = make_shared <dielectric>(1.5);
    auto material_right = make_shared <metal>(Colour(0.8, 0.6, 0.2),0.0);

    //objects
    world.add(make_shared<sphere>(Point3f(0, -100.5, -1), 100.0, material_ground));
    world.add(make_shared<sphere>(Point3f(0, 0.0, -1.0), 0.5, material_center));
    world.add(make_shared<sphere>(Point3f(-1.0, 0.0, -1.0), 0.5, material_left));
    world.add(make_shared<sphere>(Point3f(-1.0, 0.0, -1.0), -0.45, material_left));
    world.add(make_shared<sphere>(Point3f(1.0, 0.0, -1.0), 0.5, material_right));

    const Colour white(255, 255, 255);
    const Colour black(0, 0, 0);
    const Colour red(255, 0, 0);

    //const sphere Testsphere(Vec3f(screen->w * 0.5, screen->h * 0.5, 50), 50);
    //const sphere light(Vec3f(0, 0, 50), 1);

 
    double t;
    Colour pix_col(black);

    SDL_Event e;
    bool running = true;
    while (running) {

        auto t_start = std::chrono::high_resolution_clock::now();

        // clear back buffer, pixel data on surface and depth buffer (as movement)
        SDL_FillRect(screen, nullptr, SDL_MapRGB(screen->format, 0, 0, 0));
        SDL_RenderClear(renderer);

        /* source from Ryan Westwood starts here*/
        {
            t_start = std::chrono::high_resolution_clock::now();
            ThreadPool pool(std::thread::hardware_concurrency());

            int start = screen->h - 1;
            int step = screen->h / std::thread::hardware_concurrency();
            for (int y = 0; y < screen->h - 1; y++) {
                pool.Enqueue(std::bind(lineRender, screen, world, y, spp, max_depth, &cam));
            }
        }
        /*Source from Ryan Westwood ends here*/ 
        auto t_end = std::chrono::high_resolution_clock::now();
        auto passedTime = std::chrono::duration<double, std::milli>(t_end - t_start).count();
        std::cerr << "Frame render time:  " << passedTime << " ms" << std::endl;



        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, screen);
        if (texture == NULL) {
            fprintf(stderr, "CreateTextureFromSurface failed: %s\n", SDL_GetError());
            exit(1);
        }
        SDL_FreeSurface(screen);

        SDL_RenderCopyEx(renderer, texture, nullptr, nullptr,0,0,SDL_FLIP_VERTICAL);
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
