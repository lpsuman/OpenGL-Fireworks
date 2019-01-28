#ifndef PARTICLE_H_
#define PARTICLE_H_

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtx/vector_angle.hpp>

using namespace glm;

class Particle {
public:
    vec3 pos;
    float v;
    vec3 direction;
    float t;
    float decay_speed;
    float size;
    vec3 color;
    vec3 rot_axis;
    float rot_angle;

    Particle(vec3 pos, float v, vec3 direction, float decay_speed, float size, vec3 color);
};

class ParticleSource {
public:
    vec3 pos;
    float point_v;
    vec3 direction_bias;
    float point_decay_speed;
    float point_size;
    vec3 point_color;
    int birth_rate;
    int frames_until_birth;
    int point_on_spline;

    ParticleSource();
};

#endif