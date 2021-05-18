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
#include "model.h"
#include "triangles.h"
#include "bvh.h"
#include "Texture.h"
#include "rtw_stb_image.h"
#include "tgaimage.h"
#include <fstream>
#include <chrono>

#define M_PI 3.14159265359

SDL_Window* window;
SDL_Renderer* renderer;
SDL_Surface* screen;
int renderHeight =1920;
int  renderWidth  = 1080;
TGAImage image(renderHeight, renderWidth, TGAImage::RGB);
void init() {
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* window = SDL_CreateWindow(
        "Software Ray Tracer",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        (renderHeight),
        (renderWidth),
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

// method to ensure colours don’t go out of 8 bit range in RGB​
void clamp255(Vec3f& col) {
    col.x = (col.x > 255) ? 255 : (col.x < 0) ? 0 : col.x;
    col.y = (col.y > 255) ? 255 : (col.y < 0) ? 0 : col.y;
    col.z = (col.z > 255) ? 255 : (col.z < 0) ? 0 : col.z;
}


Colour ray_colour(const Ray& r,const Colour& background, const hittable& world, int depth) {
    hit_record rec;
    //if we have hit the depth limit no more light has been gathered
    if (depth <= 0)  return Colour(0, 0, 0); 
    if (!world.hit(r, 0.001, infinity, rec)) { return background; }
    Ray scattered;
    Colour attenuation;
    Colour emitted = rec.mat_ptr->emitted();
    if (!rec.mat_ptr->scatter(r, rec, attenuation, scattered))
        return emitted; 
    return attenuation * ray_colour(scattered, background, world, depth - 1);
}
void lineRender(SDL_Surface*screen, hittable_list world, int y, int spp, int max_depth, camera*cam) {
    Colour background(0, 0, 0);
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
                Vec3f unit_direction = ray.direction().normalize();
                auto t = 0.5 * (unit_direction.y + 1.0);
                background = (1.0 - t) * Colour(1.0, 1.0, 1.0) + t * Colour(0.5, 0.7, 1.0) * 255;
                //colours for every sample
                pix_col = pix_col + ray_colour(ray,background, world, max_depth);
            }
            //scale spp and gamma correct
            pix_col /= 255.f * spp;
            pix_col.x = sqrt(pix_col.x);
            pix_col.y = sqrt(pix_col.y);
            pix_col.z = sqrt(pix_col.z);
            pix_col *= 255;
            Uint32 colour = SDL_MapRGB(screen->format, pix_col.x, pix_col.y, pix_col.z);
            //used from week 2 work 
            TGAColor tgacolour(pix_col.x, pix_col.y, pix_col.z, 255);
            putpixel(screen, x, y, colour);
            image.set(x, y, tgacolour);
        }
    }

hittable_list test_scene() {
    hittable_list world;
    Model* model = new Model("table.obj");
    Model* handle = new Model("Handle.obj");
    Model* mirror = new Model("Mirror.obj");
    Model* mirrorinner = new Model("MirrorInner.obj");
    Model* glassball = new Model("Water.obj");
    Model* water = new Model("Waterball.obj");
    Model* flower = new Model("Damdelion.obj");
    Model* wall = new Model("Wall.obj");
    Model* floor = new Model("Floor.obj");
    Model* areaLight = new Model("AreaLight.obj");

    //loading table model 
    auto transform= Vec3f(0, 0, 0);
    auto mat_texture = make_shared<image_texture>("TableUvs.jpg");
    auto mat_diffuse = make_shared<lambertian>(make_shared<image_texture>("TableUvs.jpg"));
    for (uint32_t i = 0; i < model->nfaces(); i++) {
        const Vec3f& v0 = model->vert(model->face(i)[0]);
        const Vec3f& v1 = model->vert(model->face(i)[1]);
        const Vec3f& v2 = model->vert(model->face(i)[2]);

        const Vec3f& v0N = model->vnorms(model->vNorms(i)[0]);
        const Vec3f& v1N = model->vnorms(model->vNorms(i)[1]);
        const Vec3f& v2N = model->vnorms(model->vNorms(i)[2]);

        const Vec2f& UVu = model->vt(model->uvs(i)[0]);
        const Vec2f& UVy = model->vt(model->uvs(i)[1]);

        world.add(make_shared<triangle>(v0 + transform, v1 + transform, v2 + transform,v0N,v1N,v2N, UVu, UVy,mat_diffuse));
    }
    ////loading table handel 
    auto metal_diffuse = make_shared<metal>(Colour(0, 0, 0),0);
    for (uint32_t i = 0; i < handle->nfaces(); i++) {
        const Vec3f& v0 = handle->vert(handle->face(i)[0]);
        const Vec3f& v1 = handle->vert(handle->face(i)[1]);
        const Vec3f& v2 = handle->vert(handle->face(i)[2]);

        const Vec3f& v0N = handle->vnorms(handle->vNorms(i)[0]);
        const Vec3f& v1N = handle->vnorms(handle->vNorms(i)[1]);
        const Vec3f& v2N = handle->vnorms(handle->vNorms(i)[2]);

        const Vec2f& UVu = handle->vt(handle->uvs(i)[0]);
        const Vec2f& UVy = handle->vt(handle->uvs(i)[1]);

        world.add(make_shared<triangle>(v0 + transform, v1 + transform, v2 + transform, v0N, v1N, v2N,UVu,UVy, metal_diffuse));
    }
    ////loading mirror
    mat_diffuse = make_shared<lambertian>(Colour(0,1,1));
    for (uint32_t i = 0; i < mirror->nfaces(); i++) {
        const Vec3f& v0 = mirror->vert(mirror->face(i)[0]);
        const Vec3f& v1 = mirror->vert(mirror->face(i)[1]);
        const Vec3f& v2 = mirror->vert(mirror->face(i)[2]);

        const Vec3f& v0N = mirror->vnorms(mirror->vNorms(i)[0]);
        const Vec3f& v1N = mirror->vnorms(mirror->vNorms(i)[1]);
        const Vec3f& v2N = mirror->vnorms(mirror->vNorms(i)[2]);

        const Vec2f& UVu = mirror->vt(mirror->uvs(i)[0]);
        const Vec2f& UVy = mirror->vt(mirror->uvs(i)[1]);


        world.add(make_shared<triangle>(v0 + transform, v1 + transform, v2 + transform, v0N, v1N, v2N, UVu,UVy, mat_diffuse));
    }
    ////loading mirrorinner  
     metal_diffuse = make_shared<metal>(Colour(1, 1, 1), 0);
    for (uint32_t i = 0; i < mirrorinner->nfaces(); i++) {
        const Vec3f& v0 = mirrorinner->vert(mirrorinner->face(i)[0]);
        const Vec3f& v1 = mirrorinner->vert(mirrorinner->face(i)[1]);
        const Vec3f& v2 = mirrorinner->vert(mirrorinner->face(i)[2]);

        const Vec3f& v0N = mirrorinner->vnorms(mirrorinner->vNorms(i)[0]);
        const Vec3f& v1N = mirrorinner->vnorms(mirrorinner->vNorms(i)[1]);
        const Vec3f& v2N = mirrorinner->vnorms(mirrorinner->vNorms(i)[2]);

        const Vec2f& UVu = mirrorinner->vt(mirrorinner->uvs(i)[0]);
        const Vec2f& UVy = mirrorinner->vt(mirrorinner->uvs(i)[1]);

        world.add(make_shared<triangle>(v0 + transform, v1 + transform, v2 + transform, v0N, v1N, v2N, UVu,UVy, metal_diffuse));
    }
    ////loading glass ball
    auto glass_diffuse = make_shared<dielectric>(1.5);
    for (uint32_t i = 0; i < glassball->nfaces(); i++) {
        const Vec3f& v0 = glassball->vert(glassball->face(i)[0]);
        const Vec3f& v1 = glassball->vert(glassball->face(i)[1]);
        const Vec3f& v2 = glassball->vert(glassball->face(i)[2]);

        const Vec3f& v0N = glassball->vnorms(glassball->vNorms(i)[0]);
        const Vec3f& v1N = glassball->vnorms(glassball->vNorms(i)[1]);
        const Vec3f& v2N = glassball->vnorms(glassball->vNorms(i)[2]);

        const Vec2f& UVu = glassball->vt(glassball->uvs(i)[0]);
        const Vec2f& UVy = glassball->vt(glassball->uvs(i)[1]);

        world.add(make_shared<triangle>(v0 + transform, v1 + transform, v2 + transform, v0N, v1N, v2N,UVu,UVy, glass_diffuse));
    }
    ////loading water
    auto water_mat = make_shared<Water>(1.3);
    for (uint32_t i = 0; i < water->nfaces(); i++) {
        const Vec3f& v0 = water->vert(water->face(i)[0]);
        const Vec3f& v1 = water->vert(water->face(i)[1]);
        const Vec3f& v2 = water->vert(water->face(i)[2]);

        const Vec3f& v0N = water->vnorms(water->vNorms(i)[0]);
        const Vec3f& v1N = water->vnorms(water->vNorms(i)[1]);
        const Vec3f& v2N = water->vnorms(water->vNorms(i)[2]);

        const Vec2f& UVu = water->vt(water->uvs(i)[0]);
        const Vec2f& UVy = water->vt(water->uvs(i)[1]);

        world.add(make_shared<triangle>(v0 + transform, v1 + transform, v2 + transform, v0N, v1N, v2N,UVu,UVy, water_mat));
    }
    //////loading wall
    mat_diffuse = make_shared<lambertian>(Colour(0.5, 0.5, 0.5));
    for (uint32_t i = 0; i < wall->nfaces(); i++) {
        const Vec3f& v0 = wall->vert(wall->face(i)[0]);
        const Vec3f& v1 = wall->vert(wall->face(i)[1]);
        const Vec3f& v2 = wall->vert(wall->face(i)[2]);

        const Vec3f& v0N = wall->vnorms(wall->vNorms(i)[0]);
        const Vec3f& v1N = wall->vnorms(wall->vNorms(i)[1]);
        const Vec3f& v2N = wall->vnorms(wall->vNorms(i)[2]);

        const Vec2f& UVu = wall->vt(wall->uvs(i)[0]);
        const Vec2f& UVy = wall->vt(wall->uvs(i)[1]);

        world.add(make_shared<triangle>(v0 + transform, v1 + transform, v2 + transform, v0N, v1N, v2N,UVu,UVy, mat_diffuse));
    }
    ////loading floor
    mat_diffuse = make_shared<lambertian>(Colour(0.5, 0.5, 0.5));
    for (uint32_t i = 0; i < floor->nfaces(); i++) {
        const Vec3f& v0 = floor ->vert(floor->face(i)[0]);
        const Vec3f& v1 = floor->vert(floor->face(i)[1]);
        const Vec3f& v2 = floor->vert(floor->face(i)[2]);

        const Vec3f& v0N = floor->vnorms(floor->vNorms(i)[0]);
        const Vec3f& v1N = floor->vnorms(floor->vNorms(i)[1]);
        const Vec3f& v2N = floor->vnorms(floor->vNorms(i)[2]);

        const Vec2f& UVu = floor->vt(floor->uvs(i)[0]);
        const Vec2f& UVy = floor->vt(floor->uvs(i)[1]);

        world.add(make_shared<triangle>(v0 + transform, v1 + transform, v2 + transform, v0N, v1N, v2N,UVu,UVy, mat_diffuse));
    }
    ////loading flower
    mat_texture = make_shared<image_texture>("qlCc6_4K_Albedo.jpg");
    mat_diffuse = make_shared<lambertian>(mat_texture);
    for (uint32_t i = 0; i < flower->nfaces(); i++) {
        const Vec3f& v0 = flower->vert(flower->face(i)[0]);
        const Vec3f& v1 = flower->vert(flower->face(i)[1]);
        const Vec3f& v2 = flower->vert(flower->face(i)[2]);

        const Vec3f& v0N = flower->vnorms(flower->vNorms(i)[0]);
        const Vec3f& v1N = flower->vnorms(flower->vNorms(i)[1]);
        const Vec3f& v2N = flower->vnorms(flower->vNorms(i)[2]);

        const Vec2f& UVu = flower->vt(flower->uvs(i)[0]);
        const Vec2f& UVy = flower->vt(flower->uvs(i)[1]);

        world.add(make_shared<triangle>(v0 + transform, v1 + transform, v2 + transform, v0N, v1N, v2N,UVu,UVy, mat_diffuse));
    }
    ////loading arealight
    auto light_diffuse = make_shared<diffuse_light>(Colour(255,255,255));
    for (uint32_t i = 0; i < areaLight->nfaces(); i++) {
        const Vec3f& v0 = areaLight->vert(areaLight->face(i)[0]);
        const Vec3f& v1 = areaLight->vert(areaLight->face(i)[1]);
        const Vec3f& v2 = areaLight->vert(areaLight->face(i)[2]);

        const Vec3f& v0N = areaLight->vnorms(areaLight->vNorms(i)[0]);
        const Vec3f& v1N = areaLight->vnorms(areaLight->vNorms(i)[1]);
        const Vec3f& v2N = areaLight->vnorms(areaLight->vNorms(i)[2]);

        const Vec2f& UVu = areaLight->vt(areaLight->uvs(i)[0]);
        const Vec2f& UVy = areaLight->vt(areaLight->uvs(i)[1]);

        world.add(make_shared<triangle>(v0 + transform, v1 + transform, v2 + transform, v0N, v1N, v2N,UVu,UVy, light_diffuse));
    }
    return hittable_list(make_shared<bvh_node>(world)); //with bvh
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
    const int spp =1000;
    const float scale = 1.0f / spp;

    //camera (should be in main.ccp)

    Point3f lookfrom(31, 40, 29);
    Point3f lookat(0, 25, 8);
    Vec3f vup(0, 1, 0);
    auto dist_to_focus = (lookfrom - lookat).length();
    auto aperture = 0.05;
    camera cam(lookfrom,lookat,vup,35,aspect_ratio,aperture, dist_to_focus);

    //world
    hittable_list world;

    world = test_scene();

    const Colour white(255, 255, 255);
    const Colour black(0, 0, 0);
    const Colour red(255, 0, 0);
 
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

        image.flip_vertically();
        image.write_tga_file("raytracer_renderer.tga");



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
