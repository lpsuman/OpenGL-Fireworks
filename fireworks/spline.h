#ifndef SPLINE_H_
#define SPLINE_H_

#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <vector>

class Spline {
 private:
  int num_t_steps;
  float scaling_factor;
  std::vector<glm::vec3> control_points;
  int num_segments;
  glm::vec3 min_pos;
  glm::vec3 max_pos;
  bool readBSpline(const char* file_path);
  void calcSplinePoints();

 public:
  std::vector<glm::vec3> points;
  std::vector<glm::vec3> tan_points;
  std::vector<glm::vec3> tan2_points;
  std::vector<float> cumulative_distances;
  Spline(const char* file_path);
  Spline(const char* file_path, int num_t_steps, float scaling_factor);
  void drawSpline(int from_point, float line_size, glm::vec3 line_color);
  int findDistanceStep(int current_point, float target_dist);
};

#endif