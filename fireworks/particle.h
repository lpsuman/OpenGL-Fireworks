#ifndef PARTICLE_H_
#define PARTICLE_H_

#include <GL/freeglut.h>
#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <vector>

using namespace glm;

class Particle {
 public:
  vec3 pos;
  vec3 v;
  float t;
  float decay_speed;
  float size;
  vec3 color;
  vec3 rot_axis;
  float rot_angle;

  Particle(vec3 pos, vec3 v, float decay_speed, float size, vec3 color);
};

class ParticleSource {
 private:
  float accumulated_particles;

 public:
  vec3 pos;
  vec3 v;
  float point_v;
  vec3 direction_bias;
  float point_decay_speed;
  float point_size;
  vec3 point_color;
  float birth_rate;
  int point_on_spline;

  ParticleSource();
  ParticleSource(float point_v, float point_decay_speed, float point_size,
                 glm::vec3 point_color, float birth_rate);
  void generateParticles(std::vector<Particle *> *particles, float delta,
                         bool is_burst);
};

void drawParticles(const std::vector<Particle *> &particles);

void updateParticles(std::vector<Particle *> *particles, float delta_time,
                     float min_y);

void deleteParticles(std::vector<Particle *> *particles);

GLuint LoadTextureRAW(const char *filename, int wrap);

#endif