#pragma once
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <cstdio>
#include <filesystem>

/* Generate a random float */
float rand_float(float from = 0, float to = 1);

float clamp(float value, float from, float to);

enum CoolingSchedule { C_1, C_50, C_195075, LINEAR, STAIRCASE, SIGMOID, GEOMETRIC, LINEAR_REHEAT, COSINE };

extern const char *schedule_names[9];

bool LoadTextureFromFile(std::filesystem::path path, GLuint *out_texture, int *out_width, int *out_height,
                         GLubyte **out_data);
