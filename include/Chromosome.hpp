#pragma once

#include <vector>
#include <glad/glad.h>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

struct Triangle {
  glm::vec2 vs[3];
  glm::vec4 color;
};

class Chromosome {
 public:
  Chromosome() = default;
  Chromosome(const size_t size);
  Chromosome(std::vector<Triangle> triangles);

  void Mutate();
  void Draw(GLuint buffer, int image_width, int image_height);

  const std::vector<Triangle> &GetTriangles() const;
  void SetFitness(float fitness);
  float GetFitness() const;

  const Triangle &operator[](const size_t idx) const;

 private:
  enum MutationType { COLOR, ORDER, POSITION, LAST };

  std::vector<Triangle> triangles_;
  float fitness_ = INFINITY;
};