#pragma once

#include <glad/glad.h>
#include <vector>
#include <Utils.hpp>
#include <Selection.hpp>
#include <Crossover.hpp>

struct IterationResult {
  size_t iteration;
  GLuint texture;
  float best_fitness;
  float worst_fitness;
  float mean_fitness;
};

class Solver {
 public:
  Solver() = default;
  Solver(Image image, size_t pop_size, size_t genome_size, float cleansing_rate, CrossoverType crossover_type,
         SelectionType selection_type);
  IterationResult Iteration();
  void Cleanup();

  const std::vector<GLuint> &GetTextures() const;
  GLuint GetBestTexture() const;

 private:
  // parameters
  Image image_;
  size_t population_size_;
  size_t chromosome_size_;
  bool initialized_ = false;
  size_t iteration_ = 0;

  // stuff related to genetic algorithm
  void CalcFitness_();
  std::vector<Chromosome> population_;
  CrossoverStrategy *Crossover_;
  SelectionStrategy *Selection_;
  float best_fitness_ = 0;

  // Selection functions
  std::vector<Chromosome> UniformSelection_(const std::vector<Chromosome> &chromosomes);

  // OpenGL stuff
  void SetupBuffers_();
  size_t buffer_size_;
  std::vector<GLuint> buffers_;
  std::vector<GLuint> textures_;
  std::unique_ptr<GLubyte[]> cur_pixels_;
  std::unique_ptr<GLubyte[]> img_pixels_;
};