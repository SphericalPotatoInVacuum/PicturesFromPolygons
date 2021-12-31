#pragma once

#include <glad/glad.h>

#include <cstdlib>
#include <functional>
#include <memory>
#include <vector>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <Utils.hpp>
#include <Solver.hpp>

class Application {
 public:
  Application();
  void Run();

 private:
  void InitLogging();
  void InitGlfw();
  void InitGlad();
  void InitImgui();
  void SetupOpenGL();
  void Start();

  void MSE();

  void GlfwTeardown();

  const int kWidth = 800;
  const int kHeight = 600;

  GLFWwindow *window_;
  int actual_width_;
  int actual_height_;

  Image image_;
  bool running_ = false;
  Solver solver_;
};
