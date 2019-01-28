#include "fuse.h"

#include <GL/freeglut.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <algorithm>
#include <fstream>
#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>
#include <vector>

using namespace std;
using namespace glm;

int DEFAULT_SPLINE_STEPS = 100;

extern vec3* camera_focus;

Fuse::Fuse(const char *spline_file_path, float spline_line_size,
           glm::vec3 spline_color,
           std::vector<ParticleSource *> *fuse_particle_sources,
           float fuse_burst, glm::vec3 end_pos, float scale)
    : spline_line_size(spline_line_size),
      spline_color(spline_color),
      fuse_particle_sources(fuse_particle_sources),
      fuse_burst(fuse_burst) {
  spline = new Spline(spline_file_path, DEFAULT_SPLINE_STEPS, scale);

  int num_points = spline->points.size();
  vec3 spline_end_point = spline->points[num_points - 1];
  vec3 diff_pos = end_pos - spline_end_point;
  min_y = FLT_MAX;
  for (int i = 0; i < num_points; i++) {
    spline->points[i] += diff_pos;
    min_y = std::min(min_y, spline->points[i].y);
  }

  reset();
}

void Fuse::reset() {
  is_ignited = false;
  time_to_detonate = 0.0f;
  current_burn_time = 0.0f;

  for (int i = 0; i < fuse_particle_sources->size(); i++) {
    (*fuse_particle_sources)[i]->point_on_spline = 0;
    (*fuse_particle_sources)[i]->pos = spline->points[0];
  }

  deleteParticles(&particles);

  if (firework != NULL) {
    firework->reset();
  }
}

void Fuse::ignite(float fuse_time) {
  if (is_ignited) {
    return;
  }
  is_ignited = true;
  time_to_detonate = fuse_time;
  camera_focus = &((*fuse_particle_sources)[0]->pos);
}

void Fuse::updateFuse(float delta) {
  if (!is_ignited) {
    return;
  }
  current_burn_time += delta;
  float burn_time_ratio = current_burn_time / time_to_detonate;
  if (burn_time_ratio >= 1.0f) {
    if (firework != NULL) {
      firework->launch();
    }
    if (particles.size() > 0) {
      updateParticles(&particles, delta, min_y);
    } else {
      is_ignited = false;
    }
    return;
  }

  int num_points = spline->points.size();
  float current_burn_distance =
      burn_time_ratio * spline->cumulative_distances[num_points - 1];

  updateParticles(&particles, delta, min_y);

  for (int i = 0; i < fuse_particle_sources->size(); i++) {
    ParticleSource *fuse_source = (*fuse_particle_sources)[i];

    fuse_source->point_on_spline = spline->findDistanceStep(
        fuse_source->point_on_spline, current_burn_distance);

    fuse_source->pos = spline->points[fuse_source->point_on_spline];

    fuse_source->direction_bias =
        -normalize(spline->tan_points[fuse_source->point_on_spline]) *
        fuse_burst;

    fuse_source->generateParticles(&particles, delta, false);
  }
}

void Fuse::drawFuse() {
  spline->drawSpline((*fuse_particle_sources)[0]->point_on_spline,
                     spline_line_size, spline_color);
  drawParticles(particles);
}