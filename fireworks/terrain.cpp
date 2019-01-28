#include "terrain.h"

#include <GL/freeglut.h>
#include <stdlib.h>
#include <algorithm>
#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <iostream>

#include "imageloader.h"

using namespace glm;
using namespace std;

float DEFAULT_SMOOTHING = 0.5f;

const float tex_border = 0.05;
const float tex_pos[]{tex_border, 1.0f - tex_border};

void Terrain::drawTerrain(vec3 center_point, float scale, vec3 color) {
  scale /= std::max(width - 1, length - 1);

  glScalef(scale, scale, scale);
  glTranslatef(center_point.x - ((float)width - 1.0f) / 2.0f, center_point.y,
               center_point.z - ((float)length - 1.0f) / 2.0f);

  glColor3f(color.x, color.y, color.z);
  for (int z = 0; z < length - 1; z++) {
    glBegin(GL_TRIANGLE_STRIP);
    for (int x = 0; x < width; x++) {
      vec3 normal = normals[x][z];
      glNormal3f(normal.x, normal.y, normal.z);
      // glTexCoord2d(tex_pos[x % 2], tex_pos[0]);
      glTexCoord2d(tex_pos[x % 2], tex_pos[z % 2]);
      glVertex3f(x, heights[x][z], z);

      normal = normals[x][z + 1];
      glNormal3f(normal.x, normal.y, normal.z);
      // glTexCoord2d(tex_pos[x % 2], tex_pos[1]);
      glTexCoord2d(tex_pos[x % 2], tex_pos[(z + 1) % 2]);
      glVertex3f(x, heights[x][z + 1], z + 1);
    }
    glEnd();

    // glBegin(GL_TRIANGLES);
    // for (int x = 0; x < width - 1; x++) {
    //   vec3 normal00 = normals[x][z];
    //   vec3 normal01 = normals[x][z + 1];
    //   vec3 normal10 = normals[x + 1][z];
    //   vec3 normal11 = normals[x + 1][z + 1];

    //   glNormal3f(normal00.x, normal00.y, normal00.z);
    //   glTexCoord2d(tex_pos[0], tex_pos[0]);
    //   glVertex3f(x, heights[x][z], z);

    //   glNormal3f(normal01.x, normal01.y, normal01.z);
    //   glTexCoord2d(tex_pos[0], tex_pos[1]);
    //   glVertex3f(x, heights[x][z + 1], z + 1);

    //   glNormal3f(normal11.x, normal11.y, normal11.z);
    //   glTexCoord2d(tex_pos[1], tex_pos[1]);
    //   glVertex3f(x + 1, heights[x + 1][z + 1], z + 1);

    //   glNormal3f(normal00.x, normal00.y, normal00.z);
    //   glTexCoord2d(tex_pos[0], tex_pos[0]);
    //   glVertex3f(x, heights[x][z], z);

    //   glNormal3f(normal11.x, normal11.y, normal11.z);
    //   glTexCoord2d(tex_pos[1], tex_pos[1]);
    //   glVertex3f(x + 1, heights[x + 1][z + 1], z + 1);

    //   glNormal3f(normal10.x, normal10.y, normal10.z);
    //   glTexCoord2d(tex_pos[1], tex_pos[0]);
    //   glVertex3f(x + 1, heights[x + 1][z], z);
    // }
    // glEnd();
  }

  glTranslatef(-center_point.x + ((float)width - 1.0f) / 2.0f, -center_point.y,
               -center_point.z + ((float)length - 1.0f) / 2.0f);
  glScalef(1.0f / scale, 1.0f / scale, 1.0f / scale);
}

Terrain::Terrain(int width, int length, float smoothing_ratio)
    : width(width), length(length), smoothing_ratio(smoothing_ratio) {
  heights = new float *[length];
  normals = new vec3 *[length];
  for (int i = 0; i < length; i++) {
    heights[i] = new float[width];
    normals[i] = new vec3[width];
  }
}

void Terrain::calculateNormals() {
  vec3 **approx_normals = new vec3 *[length];
  for (int i = 0; i < length; i++) {
    approx_normals[i] = new vec3[width];
  }

  for (int z = 0; z < length; z++) {
    for (int x = 0; x < width; x++) {
      vec3 sum(0.0f, 0.0f, 0.0f);
      vec3 left, right, down, up;
      float current_height = heights[z][x];

      if (x > 0) {
        left = vec3(-1.0f, heights[z][x - 1] - current_height, 0.0f);
      }
      if (x < width - 1) {
        right = vec3(1.0f, heights[z][x + 1] - current_height, 0.0f);
      }
      if (z > 0) {
        down = vec3(0.0f, heights[z - 1][x] - current_height, -1.0f);
      }
      if (z < length - 1) {
        up = vec3(0.0f, heights[z + 1][x] - current_height, 1.0f);
      }

      if (x > 0 && z > 0) {
        sum += normalize(cross(down, left));
      }
      if (x > 0 && z < length - 1) {
        sum += normalize(cross(left, up));
      }
      if (x < width - 1 && z < length - 1) {
        sum += normalize(cross(up, right));
      }
      if (x < width - 1 && z > 0) {
        sum += normalize(cross(right, down));
      }

      approx_normals[z][x] = sum;
    }
  }

  for (int z = 0; z < length; z++) {
    for (int x = 0; x < width; x++) {
      vec3 sum = approx_normals[z][x];

      if (x > 0) {
        sum += approx_normals[z][x - 1] * smoothing_ratio;
      }
      if (x < width - 1) {
        sum += approx_normals[z][x + 1] * smoothing_ratio;
      }
      if (z > 0) {
        sum += approx_normals[z - 1][x] * smoothing_ratio;
      }
      if (z < length - 1) {
        sum += approx_normals[z + 1][x] * smoothing_ratio;
      }

      if (glm::length(sum) == 0.0f) {
        sum = vec3(0.0f, 1.0f, 0.0f);
      }

      normals[z][x] = sum;
    }
  }

  for (int i = 0; i < length; i++) {
    delete[] approx_normals[i];
  }
  delete[] approx_normals;
}

Terrain *loadTerrain(const char *file_path, float height_scale) {
  Image *img = loadBMP(file_path);
  int width = img->width;
  int length = img->height;
  Terrain *terrain = new Terrain(width, length, DEFAULT_SMOOTHING);

  for (int y = 0; y < length; y++) {
    for (int x = 0; x < width; x++) {
      unsigned char color = (unsigned char)img->pixels[3 * (y * width + x)];
      float height = ((color / 255.0f) - 0.5f) * height_scale;
      terrain->heights[x][y] = height;
    }
  }

  terrain->calculateNormals();
  delete img;

  return terrain;
}

Terrain::~Terrain() {
  for (int i = 0; i < length; i++) {
    delete[] heights[i];
    delete[] normals[i];
  }
  delete[] heights;
  delete[] normals;
}