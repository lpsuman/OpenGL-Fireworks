#ifndef SPLINE_H_
#define SPLINE_H_

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <vector>

using namespace std;
using namespace glm;

class Spline {
private:
    int num_t_steps;
    float scaling_factor;
    vector<vec3> control_points;
    int num_segments;
    vec3 min_pos;
    vec3 max_pos;
    bool readBSpline(char* file_path);
    void calcSplinePoints();
public:
    vector<vec3> points;
    vector<vec3> tan_points;
    vector<vec3> tan2_points;
    Spline(char* file_path);
    Spline(char* file_path, int num_t_steps, float scaling_factor);
};

#endif