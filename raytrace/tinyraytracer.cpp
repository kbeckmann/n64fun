#pragma GCC optimize("Ofast")

// Port of https://github.com/ssloy/tinyraytracer

#include "geometry.h"

extern "C" {
#include <libdragon.h>
}

#include <limits>
#include <fstream>
#include <vector>
#include <algorithm>

#define ARRAY_SIZE(x) (sizeof(x)/sizeof((x)[0]))

void *active_fb;

struct Light {
    vec3 position;
    float intensity;
};

struct Material {
    float refractive_index = 1;
    vec4 albedo = {1,0,0,0};
    vec3 diffuse_color = {0,0,0};
    float specular_exponent = 0;
};

struct Sphere {
    vec3 center;
    float radius;
    Material material;
};

float t;

bool ray_sphere_intersect(const vec3 &orig, const vec3 &dir, const Sphere &s, float &t0) {
    vec3 L = s.center - orig;
    float tca = L*dir;
    float d2 = L*L - tca*tca;
    if (d2 > s.radius*s.radius) return false;
    float thc = sqrtf(s.radius*s.radius - d2);
    t0       = tca - thc;
    float t1 = tca + thc;
    if (t0 < 1e-3) t0 = t1;  // offset the original point to avoid occlusion by the object itself
    if (t0 < 1e-3) return false;
    return true;
}

vec3 reflect(const vec3 &I, const vec3 &N) {
    return I - N*2.f*(I*N);
}

vec3 refract(const vec3 &I, const vec3 &N, const float eta_t, const float eta_i=1.f) { // Snell's law
    float cosi = - std::max(-1.f, std::min(1.f, I*N));
    if (cosi<0) return refract(I, -N, eta_i, eta_t); // if the ray comes from the inside the object, swap the air and the media
    float eta = eta_i / eta_t;
    float k = 1 - eta*eta*(1 - cosi*cosi);
    return k<0 ? vec3{1,0,0} : I*eta + N*(eta*cosi - std::sqrt(k)); // k<0 = total reflection, no ray to refract. I refract it anyways, this has no physical meaning
}

bool scene_intersect(const vec3 &orig, const vec3 &dir, const std::vector<Sphere> &spheres, vec3 &hit, vec3 &N, Material &material) {
    float spheres_dist = std::numeric_limits<float>::max();
    for (const Sphere &s : spheres) {
        float dist_i;
        if (ray_sphere_intersect(orig, dir, s, dist_i) && dist_i < spheres_dist) {
            spheres_dist = dist_i;
            hit = orig + dir*dist_i;
            N = (hit - s.center).normalize();
            material = s.material;
        }
    }

    float checkerboard_dist = std::numeric_limits<float>::max();
    if (std::abs(dir.y)>1e-3) { // avoid division by zero
        float d = -(orig.y+4)/dir.y; // the checkerboard plane has equation y = -4
        vec3 pt = orig + dir*d;
        if (d>1e-3 && fabs(pt.x)<10 && pt.z<-10 && pt.z>-30 && d<spheres_dist) {
            checkerboard_dist = d;
            hit = pt;
            N = vec3{0,1,0};
            material.diffuse_color = (int(.5*hit.x+1000) + int(.5*hit.z)) & 1 ? vec3{.3, .3, .3} : vec3{.3, .2, .1};
        }
    }
    return std::min(spheres_dist, checkerboard_dist)<1000;
}

vec3 cast_ray(const vec3 &orig, const vec3 &dir, const std::vector<Sphere> &spheres, const std::vector<Light> &lights, size_t depth=0) {
    vec3 point, N;
    Material material;

    if (depth>4 || !scene_intersect(orig, dir, spheres, point, N, material)) {
        float r = std::max(0.0f, 0.3f + dir[1]);
        return vec3{r, 0.2, 0.6}; // background color
    }

    vec3 reflect_dir = reflect(dir, N).normalize();
    vec3 refract_dir = refract(dir, N, material.refractive_index).normalize();
    vec3 reflect_color = cast_ray(point, reflect_dir, spheres, lights, depth + 1);
    vec3 refract_color = cast_ray(point, refract_dir, spheres, lights, depth + 1);

    float diffuse_light_intensity = 0, specular_light_intensity = 0;
    for (const Light light : lights) {
        vec3 light_dir      = (light.position - point).normalize();

        vec3 shadow_pt, trashnrm;
        Material trashmat;
        if (scene_intersect(point, light_dir, spheres, shadow_pt, trashnrm, trashmat) &&
                (shadow_pt-point).norm() < (light.position-point).norm()) // checking if the point lies in the shadow of the light
            continue;

        diffuse_light_intensity  += light.intensity * std::max(0.f, light_dir*N);

        // std::pow() doesn't seem to work on the device.
        // specular_light_intensity += std::pow(std::max(0.f, -reflect(-light_dir, N)*dir), material.specular_exponent)*light.intensity;
        specular_light_intensity += std::max(0.f, -reflect(-light_dir, N)*dir * 0.01f) * light.intensity;
    }
    return material.diffuse_color * diffuse_light_intensity * material.albedo[0] + vec3{1., 1., 1.}*specular_light_intensity * material.albedo[1] + reflect_color*material.albedo[2] + refract_color*material.albedo[3];
}

__attribute__((optimize("unroll-loops")))
void render(const std::vector<Sphere> &spheres, const std::vector<Light> &lights) {
    const int   scale    = 2;
    const int   width    = 320 / scale;
    const int   height   = 240 / scale;
    const int   stride   = 320;
    const float fov      = M_PI/3.;

    uint16_t *fb = (uint16_t *) active_fb;

    float pos_x = 0;
    float pos_y = -3;

    for (size_t y = 0; y < height; y++) { // actual rendering loop
        for (size_t x = 0; x < width; x++) {
            float dir_x =  (x + 0.5) -  width/2.;
            float dir_y = -(y + 0.5) + height/2.;    // this flips the image at the same time
            float dir_z = -height/(2.*tan(fov/2.));

            auto c = cast_ray(vec3{pos_x, 0, pos_y}, vec3{dir_x, dir_y, dir_z}.normalize(), spheres, lights);

            float max = std::max(c[0], std::max(c[1], c[2]));
            if (max>1) c = c*(1./max);

            uint8_t r = (uint8_t) (c[0] * 31.0f);
            uint8_t g = (uint8_t) (c[1] * 63.0f);
            uint8_t b = (uint8_t) (c[2] * 31.0f);

            uint16_t col = (b) | (g << 5) | (r << 11);

            for (int xx = 0; xx < scale; xx++) {
                for (int yy = 0; yy < scale; yy++) {
                    fb[x * scale + xx + (y * scale + yy) * stride] = col;
                }
            }
        }
    }
}


display_context_t lockVideo(int wait)
{
    display_context_t dc;

    if (wait)
        while (!(dc = display_lock()));
    else
        dc = display_lock();
    return dc;
}

void unlockVideo(display_context_t dc)
{
    if (dc)
        display_show(dc);
}

/**
 * @brief Grab the texture buffer given a display context
 *
 * @param[in] x
 *            The display context returned from #display_lock
 *
 * @return A pointer to the drawing surface for that display context.
 */
#define __get_buffer( x ) __safe_buffer[(x)-1]
extern void *__safe_buffer[3];

extern "C" void render_raytrace(void)
{
    display_context_t _dc;
    const Material      ivory  = {1.0, {0.6,  0.3, 0.1, 0.0}, {0.7, 0.0, 0.0},   50.};
    const Material      ivory2  = {1.0, {0.6,  0.3, 0.1, 0.0}, {0.0, 0.5, 0.7},   50.};
    const Material      glass = {1.5, {0.0,  0.5, 0.1, 0.8}, {7.0, 0.0, 0.0},  125.};
    // const Material red_rubber = {1.0, {0.9,  0.1, 0.0, 0.0}, {0.3, 0.1, 0.1},   10.};
    const Material     mirror = {1.0, {0.0, 10.0, 0.8, 0.0}, {1.0, 1.0, 1.0}, 1425.};

    const vec3 positions[] = {
        vec3{-3,     0,   -16},
        vec3{-1.0, -1.5,  -12},
        vec3{ 1.5, -0.5,  -18},
        vec3{ 7,    5,    -18},
    };

    std::vector<Sphere> spheres = {
        Sphere{vec3{-3,    0,   -16}, 2,      ivory},
        Sphere{vec3{-1.0, -1.5, -12}, 2,      glass},
        Sphere{vec3{ 1.5, -0.5, -18}, 1.5,     ivory2},
        Sphere{vec3{ 7,    5,   -18}, 4,     mirror}
    };

    std::vector<Light> lights = {
        {{-20, 20,  20}, 1.5},
        {{ 30, 50, -25}, 1.8},
        {{ 30, 20,  30}, 1.7}
    };

    float alpha = 0;
    vec3 center = vec3{0, 0, -16};

    _dc = lockVideo(1);

    active_fb = (void *) __get_buffer(_dc);

    unlockVideo(_dc);

    while (1) {
        alpha += 0.05f;

        // Rotate objects around a point

        float c = cosf(alpha);
        float s = sinf(alpha);

        for (size_t i = 0; i < ARRAY_SIZE(positions); i++) {
            vec3 p = positions[i] - center;

            vec3 new_p = {
                p.x * c - p.z * s + center.x,
                p.y,
                p.x * s + p.z * c + center.z
            };

            spheres[i].center = new_p;
        }

        // _dc = lockVideo(1);

        // active_fb = (void *) __get_buffer(_dc);

        render(spheres, lights);

        // unlockVideo(_dc);
    }
}

