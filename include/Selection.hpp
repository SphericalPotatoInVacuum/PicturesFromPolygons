#pragma once

#include <Chromosome.hpp>
#include <vector>

enum SelectionType {
  FITNESS_PROPORTIONATE_SELECTION,
  STOCHASTIC_UNIVERSAL_SAMPLING,
  TOURNAMENT_SELECTION,
  TRUNCATION_SELECTION
};

extern const char *selection_type_names[4];

class SelectionStrategy {
 public:
  SelectionStrategy() = default;
  SelectionStrategy(const float cleansing_rate);
  virtual std::vector<Chromosome> operator()(const std::vector<Chromosome> &) = 0;

 protected:
  float cleansing_rate_ = 1.0f;
};

class FitnessPropotionateSelection : public SelectionStrategy {
 public:
  using SelectionStrategy::SelectionStrategy;
  std::vector<Chromosome> operator()(const std::vector<Chromosome> &) override;
};

class StochasticUniversalSampling : public SelectionStrategy {
 public:
  using SelectionStrategy::SelectionStrategy;
  std::vector<Chromosome> operator()(const std::vector<Chromosome> &) override;
};

class TournamentSelection : public SelectionStrategy {
 public:
  using SelectionStrategy::SelectionStrategy;
  std::vector<Chromosome> operator()(const std::vector<Chromosome> &) override;
};

class TruncationSelection : public SelectionStrategy {
 public:
  using SelectionStrategy::SelectionStrategy;
  std::vector<Chromosome> operator()(const std::vector<Chromosome> &) override;
};