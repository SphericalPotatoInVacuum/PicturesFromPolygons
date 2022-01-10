#include <Solver.hpp>
#include <Chromosome.hpp>
#include <Utils.hpp>
#include <plog/Log.h>
#include <algorithm>

Solver::Solver(Image image, size_t population_size, size_t chromosome_size, float cleansing_rate,
               CrossoverType crossover_type, SelectionType selection_type)
    : image_(image),
      population_size_(population_size),
      chromosome_size_(chromosome_size),
      initialized_(true),
      buffers_(population_size),
      textures_(population_size),
      buffer_size_(4 * image.width * image.height),
      cur_pixels_(std::make_unique<GLubyte[]>(buffer_size_)),
      img_pixels_(std::make_unique<GLubyte[]>(buffer_size_)) {
  switch (crossover_type) {
    case ONE_POINT: {
      Crossover_ = new OnePointCrossoverStrategy();
      break;
    }
    case TWO_POINT: {
      Crossover_ = new TwoPointCrossoverStrategy();
      break;
    }
    case UNIFORM: {
      Crossover_ = new UniformCrossoverStrategy();
      break;
    }
    case NONE: {
      Crossover_ = new NoneCrossoverStrategy();
      break;
    }
  }
  switch (selection_type) {
    case FITNESS_PROPORTIONATE_SELECTION: {
      Selection_ = new FitnessPropotionateSelection(cleansing_rate);
      break;
    }
    case STOCHASTIC_UNIVERSAL_SAMPLING: {
      Selection_ = new StochasticUniversalSampling(cleansing_rate);
      break;
    }
    case TOURNAMENT_SELECTION: {
      Selection_ = new TournamentSelection(cleansing_rate);
      break;
    }
    case TRUNCATION_SELECTION: {
      Selection_ = new TruncationSelection(cleansing_rate);
      break;
    }
  }

  glGetTextureImage(image.texture, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer_size_, img_pixels_.get());
  SetupBuffers_();

  population_.reserve(population_size);
  for (size_t i = 0; i < population_size; ++i) {
    population_.emplace_back(Chromosome(chromosome_size));
    population_[i].Draw(buffers_[i], image.width, image.height);
  }
  CalcFitness_();
}

IterationResult Solver::Iteration() {
  IterationResult result = {0};
  result.iteration = ++iteration_;
  result.best_fitness = 0;
  result.worst_fitness = INFINITY;

  // generate new populaiton
  std::vector<Chromosome> parents = (*Selection_)(population_);
  for (size_t i = 0; i < population_size_; ++i) {
    int idx1 = rand() % parents.size();
    int idx2 = rand() % (parents.size() - 1);
    if (idx2 >= idx1) {
      ++idx2;
    }
    population_[i] = (*Crossover_)(parents[idx1].GetTriangles(), parents[idx2].GetTriangles());
    population_[i].Mutate();
    population_[i].Draw(buffers_[i], image_.width, image_.height);
  }
  CalcFitness_();
  for (size_t i = 0; i < population_size_; ++i) {
    float fitness = population_[i].GetFitness();
    if (fitness > result.best_fitness) {
      result.best_fitness = fitness;
      result.texture = textures_[i];
    }
    if (fitness > best_fitness_) {
      glBlitNamedFramebuffer(buffers_[i], buffers_[population_size_], 0, 0, image_.width, image_.height, 0, 0,
                             image_.width, image_.height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
      best_fitness_ = fitness;
    }
    result.best_fitness = std::max(result.best_fitness, fitness);
    result.worst_fitness = std::min(result.worst_fitness, fitness);
    result.mean_fitness += fitness;
  }
  result.mean_fitness /= population_size_;

  return result;
}

void Solver::Cleanup() {
  if (initialized_) {
    PLOGI << "Deleting buffers for object " << this;
    glDeleteBuffers(population_size_, buffers_.data());
    glDeleteTextures(population_size_, textures_.data());
    delete Selection_;
    delete Crossover_;
  } else {
    PLOGI << "Nothing to clean up";
  }
}

const std::vector<GLuint> &Solver::GetTextures() const {
  return textures_;
}

GLuint Solver::GetBestTexture() const {
  if (initialized_) {
    return textures_[population_size_];
  } else {
    return -1;
  }
}

void Solver::CalcFitness_() {
  for (size_t i = 0; i < population_size_; ++i) {
    glGetTextureImage(textures_[i], 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer_size_, cur_pixels_.get());
    uint64_t se = 0;
    int diff = 0;
    for (size_t j = 0; j < buffer_size_; ++j) {
      diff = cur_pixels_[j] - img_pixels_[j];
      se += diff * diff;
    }
    double mse = static_cast<double>(se) / static_cast<double>(buffer_size_);
    population_[i].SetFitness(static_cast<double>(buffer_size_) / mse);
  }
}

void Solver::SetupBuffers_() {
  PLOGI << "Setting up buffers for object " << this;
  glGenFramebuffers(population_size_ + 1, buffers_.data());
  glGenTextures(population_size_ + 1, textures_.data());

  for (size_t i = 0; i <= population_size_; ++i) {
    glBindFramebuffer(GL_FRAMEBUFFER, buffers_[i]);
    glBindTexture(GL_TEXTURE_2D, textures_[i]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_.width, image_.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, textures_[i], 0);
    GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, DrawBuffers);
  }
  glBindBuffer(GL_FRAMEBUFFER, 0);
}