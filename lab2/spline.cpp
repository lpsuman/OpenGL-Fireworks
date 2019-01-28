#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>
#include <iterator>
#include "spline.h"

#define DEFAULT_NUM_T_STEPS 100
#define DEFAULT_SCALING_FACTOR 10.0f

using namespace std;
using namespace glm;

template<typename Out>
void split(const string &s, char delim, Out result);
vector<string> split(const string &s, char delim);

mat4x4 B_3 = transpose(mat4x4(
    -1, 3, -3, 1,
    3, -6, 3, 0,
    -3, 0, 3, 0,
    1, 4, 1, 0
    ));

mat4x3 B_2 = transpose(mat3x4(
    -1, 3, -3, 1,
    2, -4, 2, 0,
    -1, 0, 1, 0
    ));

mat4x2 B_1 = transpose(mat2x4(
    -1, 3, -3, 1,
    1, -2, 1, 0
    ));

Spline::Spline(char* file_path, int num_t_steps, float scaling_factor):
        num_t_steps(num_t_steps), scaling_factor(scaling_factor) {
    readBSpline(file_path);
    calcSplinePoints();
}

Spline::Spline(char* file_path): num_t_steps(DEFAULT_NUM_T_STEPS), scaling_factor(DEFAULT_SCALING_FACTOR) {
    readBSpline(file_path);
    calcSplinePoints();
}

bool Spline::readBSpline(char* file_path) {
    ifstream file(file_path);
    if (!file.is_open()) {
        return false;
    }

    min_pos = vec3();
    max_pos = vec3();
    float max_coord = -1000.0f;

    string line;
    while (getline(file, line)) {
        if (line.empty()) continue;
        vector<string> lineParts = split(line, ' ');
        if (lineParts.size() != 3) continue;
        
        vec3 v = vec3();
        v.x = strtod(lineParts[0].c_str(), NULL);
        v.y = strtod(lineParts[1].c_str(), NULL);
        v.z = strtod(lineParts[2].c_str(), NULL);
        control_points.push_back(v);

        max_coord = std::max(max_coord, std::max(std::abs(v.x), std::max(std::abs(v.y), std::abs(v.z))));

        min_pos.x = std::min(min_pos.x, v.x);
        min_pos.y = std::min(min_pos.y, v.y);
        min_pos.z = std::min(min_pos.z, v.z);

        max_pos.x = std::max(max_pos.x, v.x);
        max_pos.y = std::max(max_pos.y, v.y);
        max_pos.z = std::max(max_pos.z, v.z);
    }
    num_segments = control_points.size() - 3;

    if (num_segments < 1) {
        cout << "At least 4 points are required for a B-spline." << endl;
        return false;
    }

    for (int i = 0; i < control_points.size(); i++) {
        control_points[i] += min_pos;
        if (control_points[i].z > 0.0f) {
            control_points[i].z = -control_points[i].z;
        }

        // control_points[i] -= (max_pos - min_pos) / 2.0f;

        control_points[i] *= scaling_factor / max_coord;
    }

    return true;
}

void Spline::calcSplinePoints() {
    for (int i = 0; i < num_segments; i++) {
        mat4x3 R_T = mat4x3();
        for (int j = 0; j < 4; j++) {
            R_T[j] = control_points[i + j];
        }
        mat3x4 R = transpose(R_T);

        for (int j = 0; j < num_t_steps; j++) {
            float t = j / float(num_t_steps);

            vec4 T_3 = vec4(pow(t, 3), t * t, t, 1);
            vec4 temp = T_3 * B_3;
            vec3 point = temp * R * (1 / 6.0f);
            points.push_back(point);

            vec3 T_2 = vec3(t * t, t, 1);
            temp = T_2 * B_2;
            vec3 tan_point = temp * R * 0.5f;
            tan_points.push_back(tan_point);

            vec2 T_1 = vec2(t, 1);
            temp = T_1 * B_1;
            vec3 tan2_point = temp * R;
            tan2_points.push_back(tan2_point);
        }
    }
}

template<typename Out>
void split(const string &s, char delim, Out result) {
    stringstream ss;
    ss.str(s);
    string item;
    while (getline(ss, item, delim)) {
        if (!item.empty()) {
            *(result++) = item;
        }
    }
}

vector<string> split(const string &s, char delim) {
    vector<string> elems;
    split(s, delim, back_inserter(elems));
    return elems;
}