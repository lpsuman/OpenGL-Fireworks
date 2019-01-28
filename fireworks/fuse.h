#ifndef FUSE_H_
#define FUSE_H_

#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <vector>

#include "firework.h"
#include "particle.h"
#include "spline.h"

class Fuse {
 private:
  Spline *spline;
  float spline_line_size;
  glm::vec3 spline_color;
  std::vector<ParticleSource *> *fuse_particle_sources;
  float fuse_burst;
  std::vector<Particle *> particles;
  bool is_ignited;
  float time_to_detonate;
  float current_burn_time;

 public:
  float min_y;
  Firework *firework;
  Fuse(const char *spline_file_path, float spline_line_size, glm::vec3 spline_color,
       std::vector<ParticleSource *> *fuse_particle_sources, float fuse_burst,
       glm::vec3 end_pos, float scale);
  void reset();
  void ignite(float fuse_time);
  void updateFuse(float delta);
  void drawFuse();
};

#endif