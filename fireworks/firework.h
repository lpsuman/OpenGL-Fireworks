#ifndef FIREWORK_H_
#define FIREWORK_H_

#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <vector>

#include "particle.h"

class Firework {
 private:
  glm::vec3 position;
  glm::vec3 rocket_speed;
  float rocket_acceleration;
  float delay;
  float speed_threshold;
  std::vector<ParticleSource *> *fuse_sources;
  std::vector<ParticleSource *> *sources;
  float min_y;
  float fuse_burst;
  std::vector<Particle *> particles;
  Particle trajectory_particle;
  float elapsed;
  float is_running;
  float is_flying;
  float is_blown;
  float prev_height;

 public:
  Firework(glm::vec3 position, glm::vec3 rocket_speed, float rocket_acceleration, float delay, float speed_threshold,
           std::vector<ParticleSource *> *fuse_sources, std::vector<ParticleSource *> *sources, float min_y, float fuse_burst);
  void launch();
  void reset();
  void updateFirework(float delta);
  void drawFirework();
};

#endif