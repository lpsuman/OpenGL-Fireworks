#include "firework.h"

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

extern vec3 *camera_focus;

Firework::Firework(glm::vec3 position, glm::vec3 rocket_speed,
                   float rocket_acceleration, float delay,
                   float speed_threshold,
                   std::vector<ParticleSource *> *fuse_sources,
                   std::vector<ParticleSource *> *sources, float min_y,
                   float fuse_burst)
    : position(position),
      rocket_speed(rocket_speed),
      rocket_acceleration(rocket_acceleration),
      delay(delay),
      speed_threshold(speed_threshold),
      fuse_sources(fuse_sources),
      sources(sources),
      min_y(min_y),
      fuse_burst(fuse_burst),
      trajectory_particle(position, rocket_speed, 0.001f, 0.001f,
                          vec3(1.0f, 1.0f, 0.0f)) {
  is_blown = false;
  reset();
}

void Firework::launch() {
  if (is_running) {
    return;
  }
  is_running = true;
  camera_focus = &(trajectory_particle.pos);
  trajectory_particle.pos = position;
  trajectory_particle.v = rocket_speed;
}

void Firework::reset() {
  trajectory_particle.pos = position;
  trajectory_particle.v = rocket_speed;
  elapsed = 0.0f;
  is_running = false;
  if (is_blown) {
    deleteParticles(&particles);
  } else if (is_flying) {
    particles.clear();
  }
  is_flying = false;
  is_blown = false;
}

void Firework::updateFirework(float delta) {
  if (!is_running) {
    return;
  }
  elapsed += delta;

  if (!is_flying && !is_blown && elapsed >= delay) {
    is_flying = true;
    particles.push_back(&trajectory_particle);
    prev_height = -FLT_MAX;
  }

  if (is_flying) {
    if (elapsed <= 1.5f * delay) {
      trajectory_particle.v +=
          trajectory_particle.v * rocket_acceleration * delta;
    }

    for (int i = 0; i < fuse_sources->size(); i++) {
      ParticleSource *fuse_source = (*fuse_sources)[i];
      fuse_source->pos = trajectory_particle.pos;

      fuse_source->direction_bias =
          -normalize(trajectory_particle.v) * fuse_burst;

      fuse_source->generateParticles(&particles, delta, false);
    }

    if (elapsed >= 3 * delay) {
      is_flying = false;
      is_blown = true;
      particles.clear();
      for (int i = 0; i < sources->size(); i++) {
        ParticleSource *source = (*sources)[i];
        source->pos = trajectory_particle.pos;
        // source->direction_bias = trajectory_particle.v;
        source->generateParticles(&particles, delta, true);
      }
    }

    prev_height = trajectory_particle.pos.y;
  }

  if (is_flying || (is_blown && particles.size() > 0)) {
    updateParticles(&particles, delta, min_y);
  }
}

void Firework::drawFirework() {
  if (!is_running) {
    return;
  }

  if (is_flying || (is_blown && particles.size() > 0)) {
    drawParticles(particles);
  }
}