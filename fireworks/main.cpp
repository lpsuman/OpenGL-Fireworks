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

#include "control.h"
#include "firework.h"
#include "fuse.h"
#include "particle.h"
#include "spline.h"
#include "terrain.h"

using namespace std;
using namespace glm;

vec3 line_color = vec3(0.25f, 0.25f, 0.25f);
vec4 background_color = vec4(0.0f, 0.0f, 0.0f, 0.0f);
vec3 direction_color = vec3(0.0f, 1.0f, 0.0f);

int temp = 200;

char window_title[] = "Fireworks";
int window_width = 800, window_height = 800;
int window_pos_x = 100, window_pos_y = 100;
int refresh_rate_millis = 16;

Terrain* terrain;
float height_scale = 5.0f;
float terrain_scale = 300.0f;
vec3 terrain_center = vec3(0.0f, 0.0f, 0.0f);
vec3 terrain_color = vec3(0.3f, 0.9f, 0.0f);

Spline* spline;
GLuint particle_texture;
GLuint terrain_texture;
vector<Particle*> particles;
vector<ParticleSource*> sources;

Fuse* fuse;
vec3 fuse_end_pos = vec3(0.0f, -7.0f, 0.0f);
float fuse_scale = 10.0f;
float fuse_burst = 3.0f;
float fuse_time = 10.0f;
float fuse_cord_size = 10.0f;
vec3 fuse_cord_color = vec3(0.70f, 0.39f, 0.11f);  // light brown
vector<ParticleSource*> fuse_particle_sources;
float fuse_particle_speed = 10.0f;
float fuse_particle_decay = 0.2f;
float fuse_particle_size = 0.2f;
float fuse_particle_birth_rate = 300.0f;
vec3 fuse_red = vec3(1.0f, 0.0f, 0.0f);
vec3 fuse_yellow = vec3(1.0f, 1.0f, 0.0f);
float fuse_red_to_yellow_ratio = 0.3f;

Firework* firework;
vec3 rocket_speed = vec3(0.0f, 1.0f, 0.0f);
extern float gravity_power;
float rocket_acceleration = 3 * gravity_power;
float delay = 2.0f;
float speed_threshold = 0.1f;
vector<ParticleSource*> firework_sources;
float firework_density = 1000.0f;

extern vec3* camera_focus;
bool is_paused = false;
bool is_spawning = true;
bool show_grid = false;
bool show_spline = true;
bool show_terrain = true;

void myDisplay();
void myReshape(int width, int height);
void initGL(char** argv);
void timer(int value);

int main(int argc, char** argv) {
  terrain = loadTerrain("./textures/heightmap.bmp", height_scale);

  srand(time(NULL));

  ParticleSource* red_source = new ParticleSource(
      fuse_particle_speed, fuse_particle_decay, fuse_particle_size, fuse_red,
      fuse_red_to_yellow_ratio * fuse_particle_birth_rate);
  fuse_particle_sources.push_back(red_source);

  ParticleSource* yellow_source = new ParticleSource(
      fuse_particle_speed, fuse_particle_decay, fuse_particle_size, fuse_yellow,
      (1.0f - fuse_red_to_yellow_ratio) * fuse_particle_birth_rate);
  fuse_particle_sources.push_back(yellow_source);

  fuse = new Fuse("./splines/fuse.txt", fuse_cord_size, fuse_cord_color,
                  &fuse_particle_sources, fuse_burst, fuse_end_pos, fuse_scale);

  firework_sources.push_back(
      new ParticleSource(fuse_particle_speed * 3.0f, fuse_particle_decay,
                         fuse_particle_size * 10.0f, fuse_yellow,
      firework_density));

  firework_sources.push_back(
      new ParticleSource(fuse_particle_speed * 3.0f, fuse_particle_decay,
                         fuse_particle_size * 10.0f, vec3(0.5f, 0.5f, 1.0f),
      firework_density / 2.0f));

  firework =
      new Firework(fuse_end_pos, rocket_speed, rocket_acceleration, delay,
                   speed_threshold, &fuse_particle_sources, &firework_sources, fuse->min_y, fuse_burst);

  fuse->firework = firework;
  fuse->ignite(fuse_time);

  print_controls_info();

  glutInit(&argc, argv);
  initGL(argv);

  glutMainLoop();

  return 0;
}

float rand_float(float lower, float upper) {
  return lower + (upper - lower) * (rand() / RAND_MAX);
}

void timer(int value) {
  if (!is_paused) {
    float delta_time = refresh_rate_millis / 1000.0f;

    fuse->updateFuse(delta_time);

    firework->updateFirework(delta_time);
  }
  glutPostRedisplay();
  glutTimerFunc(refresh_rate_millis, timer, 0);
}

void myDisplay() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  updateCamera();

  if (show_terrain) {
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, terrain_texture);
    glEnable(GL_DEPTH_TEST);
    terrain->drawTerrain(terrain_center, terrain_scale, terrain_color);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_TEXTURE_2D);
  }

  if (show_grid) {
    draw_horizon_grid();
  }

  fuse->drawFuse();

  firework->drawFirework();

  glutSwapBuffers();
}

void initGL(char** argv) {
  // glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  glutInitWindowSize(window_width, window_height);
  glutInitWindowPosition(window_pos_x, window_pos_y);
  glutCreateWindow(window_title);

  glutDisplayFunc(myDisplay);
  glutReshapeFunc(myReshape);
  glutKeyboardFunc(myKeyboard);
  glutMouseFunc(mouseButton);
  glutMotionFunc(mouseMove);

  particle_texture = LoadTextureRAW("./textures/cestica.bmp", 0);
  terrain_texture = LoadTextureRAW("./textures/grass.bmp", 0);

  glEnable(GL_COLOR_MATERIAL);
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glEnable(GL_NORMALIZE);
  glShadeModel(GL_SMOOTH);
  glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
  glEnable(GL_LINE_SMOOTH);

  glBlendFunc(GL_SRC_ALPHA, GL_ONE);

  glClearColor(background_color.x, background_color.y, background_color.z,
               background_color.w);
  glPointSize(1.0);

  glutTimerFunc(0, timer, 0);
}

void myReshape(GLsizei width, GLsizei height) {
  if (height == 0) height = 1;
  GLfloat aspect = (GLfloat)width / (GLfloat)height;

  glViewport(0, 0, width, height);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(45.0f, aspect, 1.0f, terrain_scale);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glLoadIdentity();
  glClear(GL_COLOR_BUFFER_BIT);
}
