#define _USE_MATH_DEFINES

#include "particle.h"

#include <GL/freeglut.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>
#include <vector>

using namespace glm;
using namespace std;

extern GLuint particle_texture;
extern vec3 camera_position;

int DEFAULT_V = 5.0f;
vec3 DIRECTION_BIAS = vec3(0.0f, 1.0f, 0.0f);
float DEFAULT_DECAY_SPEED = 0.3f;
float DEFAULT_POINT_SIZE = 0.1f;
vec3 DEFAULT_PARTICLE_COLOR = vec3(1.0f, 1.0f, 0.0f);
float DEFAULT_BIRTH_RATE = 30.0f;

float gravity_power = 1.62f;
vec3 gravity = vec3(0.0f, -gravity_power, 0.0f);

Particle::Particle(vec3 pos, vec3 v, float decay_speed, float size, vec3 color)
    : pos(pos), v(v), decay_speed(decay_speed), size(size), color(color) {
  t = 0.0f;
}

ParticleSource::ParticleSource() {
  point_v = DEFAULT_V;
  direction_bias = DIRECTION_BIAS;
  point_decay_speed = DEFAULT_DECAY_SPEED;
  point_size = DEFAULT_POINT_SIZE;
  point_color = DEFAULT_PARTICLE_COLOR;
  birth_rate = DEFAULT_BIRTH_RATE;
  point_on_spline = 0;
}

ParticleSource::ParticleSource(float point_v, float point_decay_speed,
                               float point_size, glm::vec3 point_color,
                               float birth_rate)
    : point_v(point_v),
      point_decay_speed(point_decay_speed),
      point_size(point_size),
      point_color(point_color),
      birth_rate(birth_rate) {
  point_on_spline = 0;
}

void drawParticles(const std::vector<Particle *> &particles) {
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, particle_texture);
  glEnable(GL_BLEND);
  for (int i = 0; i < particles.size(); i++) {
    Particle *p = particles[i];
    glColor3f(p->color.x, p->color.y, p->color.z);
    glTranslatef(p->pos.x, p->pos.y, p->pos.z);
    glRotatef(p->rot_angle, p->rot_axis.x, p->rot_axis.y, p->rot_axis.z);
    glBegin(GL_QUADS);

    float size = p->size * (1 - p->t);

    glTexCoord2d(0.0, 0.0);
    glVertex3f(-size, -size, 0.0);
    glTexCoord2d(1.0, 0.0);
    glVertex3f(-size, +size, 0.0);
    glTexCoord2d(1.0, 1.0);
    glVertex3f(+size, +size, 0.0);
    glTexCoord2d(0.0, 1.0);
    glVertex3f(+size, -size, 0.0);

    glEnd();
    glRotatef(-p->rot_angle, p->rot_axis.x, p->rot_axis.y, p->rot_axis.z);
    glTranslatef(-p->pos.x, -p->pos.y, -p->pos.z);
  }
  glDisable(GL_BLEND);
  glDisable(GL_TEXTURE_2D);
}

void updateParticles(vector<Particle *> *particles, float delta_time,
                     float min_y) {
  for (int j = particles->size() - 1; j >= 0; j--) {
    Particle *particle = (*particles)[j];

    particle->v += gravity * delta_time;
    particle->pos += particle->v * delta_time;
    particle->pos.y = std::max(particle->pos.y, min_y);
    particle->t += particle->decay_speed * delta_time;

    vec3 s = vec3(0.0f, 0.0f, 1.0f);
    vec3 e = particle->pos - camera_position;
    particle->rot_axis = cross(s, e);
    float rotation_angle = acos(dot(s, e) / (length(s) * length(e)));
    particle->rot_angle = 360 * (rotation_angle / (2 * M_PI));

    if (particle->t >= 1.0f) {
      particles->erase(particles->begin() + j);
      delete particle;
    }
  }
}

float randomFloat(float value, float variance) {
  float min_value = (1 - variance) * value;
  float max_value = (1 + variance) * value;
  return min_value +
         (float)(rand() / ((float)RAND_MAX + 1.0f)) * (max_value - min_value);
}

void ParticleSource::generateParticles(vector<Particle *> *particles,
                                       float delta, bool is_burst) {
  if (is_burst) {
    accumulated_particles = birth_rate;
  } else {
    accumulated_particles += randomFloat(delta * birth_rate, 0.3f);
  }

  while (accumulated_particles >= 1.0f) {
    accumulated_particles -= 1.0f;
    float x, y, z, norm;
    x = (rand() % 2000 - 1000) / 1000.0f;
    y = (rand() % 2000 - 1000) / 1000.0f;
    z = (rand() % 2000 - 1000) / 1000.0f;
    norm = pow(pow(x, 2.0f) + pow(y, 2.0f) + pow(z, 2.0f), 0.5f);
    vec3 rand_direction = vec3(x / norm, y / norm, z / norm);

    Particle *particle =
        new Particle(pos,
                     normalize(rand_direction + direction_bias) * point_v *
                         randomFloat(1.0f, 0.25f),
                     point_decay_speed * randomFloat(1.0f, 0.25f),
                     point_size * randomFloat(1.0f, 0.25f), point_color);

    particles->push_back(particle);
  }
}

void deleteParticles(vector<Particle *> *particles) {
  for (int i = 0; i < particles->size(); i++) {
    delete (*particles)[i];
  }
  particles->clear();
}

// load a 256x256 RGB .RAW file as a texture
GLuint LoadTextureRAW(const char *filename, int wrap) {
  GLuint texture;
  int width, height;
  BYTE *data;
  FILE *file;
  file = fopen(filename, "rb");
  if (file == NULL) {
    return 0;
  }

  width = 256;
  height = 256;
  data = (BYTE *)malloc(width * height * 3);
  fread(data, width * height * 3, 1, file);
  fclose(file);

  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);

  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                  GL_LINEAR_MIPMAP_NEAREST);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                  wrap ? GL_REPEAT : GL_CLAMP);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
                  wrap ? GL_REPEAT : GL_CLAMP);
  gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, width, height, GL_RGB,
                    GL_UNSIGNED_BYTE, data);

  free(data);

  return texture;
}