#pragma once
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include <cstdio>
#include <filesystem>

/**
 * @brief Generate a random float
 *
 * @param from lower bound
 * @param to upper bound
 * @return float
 */
float rand_float(float from = 0, float to = 1);

/**
 * @brief Clamp a given value between boundariess
 *
 * @param value
 * @param from
 * @param to
 * @return float
 */
float clamp(float value, float from, float to);

struct Image {
  GLuint texture = -1;
  int width = 0;
  int height = 0;
};

bool LoadTextureFromFile(std::filesystem::path path, Image &texture);
