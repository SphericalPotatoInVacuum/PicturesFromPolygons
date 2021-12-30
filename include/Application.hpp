#pragma once

#include <cstdlib>
#include <functional>
#include <memory>
#include <vector>
#include "Utils.hpp"

#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

struct Vertex {
  float x;
  float y;
};

struct Triangle {
  Vertex vs[3];
  float color[4];
};

typedef std::function<float(int)> ScheduleFunc;

class Application {
 public:
  Application();
  void Run();

 private:
  enum MutationType { COLOR, ORDER, POSITION, LAST };
  void InitGlfw();
  void InitGlad();
  void InitImgui();
  void SetupOpenGL();
  void SetupNewTexture();
  void Mutate();
  void DrawTriangles(const std::vector<Triangle> &triangles, GLuint texture, std::unique_ptr<GLubyte[]> &pixels);
  void StartAnnealing();
  ScheduleFunc GetScheduleFunc(CoolingSchedule schedule);

  void MSE();

  void GlfwTeardown();

  const int kWidth = 800;
  const int kHeight = 600;

  GLFWwindow *window_;
  int actual_width_;
  int actual_height_;

  size_t triangle_count_;
  ScheduleFunc Schedule_;

  std::vector<Triangle> best_triangles_;
  std::unique_ptr<GLubyte[]> best_pixels_;
  GLuint best_buffer_name_ = 0;
  GLuint best_texture_ = 0;
  double best_mse_ = 255 * 4;

  std::vector<Triangle> cur_triangles_;
  std::unique_ptr<GLubyte[]> cur_pixels_;
  GLuint cur_buffer_name_ = 0;
  GLuint cur_texture_ = 0;
  double cur_mse_ = 255 * 4;

  std::unique_ptr<GLubyte[]> image_pixels_;
  GLuint image_texture_ = 0;
  int image_width_ = 0;
  int image_height_ = 0;

  bool annealing_ = false;
  size_t iteration_ = 0;
};
