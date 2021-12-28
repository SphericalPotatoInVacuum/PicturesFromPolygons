#include <stdlib.h>

#include <Utils.hpp>
#include <glad/glad.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

float rand_float(float from, float to) {
  float r = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
  r = r * (to - from) + from;
  return r;
}

float clamp(float value, float from, float to) {
  if (value < from) value = from;
  if (value > to) value = to;
  return value;
}

const char *schedule_names[9] = {"Geman & Geman, c = 1",
                                 "Geman & Geman, c = 50",
                                 "Geman & Geman, c = 195075",
                                 "Linear",
                                 "Staircase",
                                 "Sigmoid",
                                 "Geometric",
                                 "Linear with reheat",
                                 "Cosine"};

bool LoadTextureFromFile(const char *filename, GLuint *out_texture, int *out_width, int *out_height,
                         GLubyte **out_data) {
  int image_width = 0;
  int image_height = 0;
  unsigned char *image_data = stbi_load(filename, &image_width, &image_height, NULL, 4);
  if (image_data == NULL) {
    return false;
  }

  GLuint image_texture;
  glGenTextures(1, &image_texture);
  glBindTexture(GL_TEXTURE_2D, image_texture);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);

  *out_texture = image_texture;
  *out_width = image_width;
  *out_height = image_height;
  *out_data = image_data;
  return true;
}
