#ifndef TERRAIN_H_
#define TERRAIN_H_

#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <iostream>
#include <stdlib.h>

#include "imageloader.h"

extern float DEFAULT_SMOOTHING;

class Terrain {
  public:
    int width;
    int length;
    float smoothing_ratio;
    float** heights;
    glm::vec3** normals;

    Terrain(int w, int l, float smoothing);
    void calculateNormals();
    void drawTerrain(glm::vec3 center_point, float scale, glm::vec3 color);
    ~Terrain();
};

Terrain *loadTerrain(const char *file_path, float height_scale);

#endif