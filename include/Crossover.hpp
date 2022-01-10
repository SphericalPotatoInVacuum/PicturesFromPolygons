#pragma once

#include <Chromosome.hpp>

enum CrossoverType { ONE_POINT, TWO_POINT, UNIFORM, NONE };

extern const char *crossover_type_names[4];

class CrossoverStrategy {
 public:
  virtual Chromosome operator()(const Chromosome &child1, const Chromosome &child2) = 0;
};

class OnePointCrossoverStrategy : public CrossoverStrategy {
 public:
  Chromosome operator()(const Chromosome &child1, const Chromosome &child2) override;
};

class TwoPointCrossoverStrategy : public CrossoverStrategy {
 public:
  Chromosome operator()(const Chromosome &child1, const Chromosome &child2) override;
};

class UniformCrossoverStrategy : public CrossoverStrategy {
 public:
  Chromosome operator()(const Chromosome &child1, const Chromosome &child2) override;
};

class NoneCrossoverStrategy : public CrossoverStrategy {
 public:
  Chromosome operator()(const Chromosome &child1, const Chromosome &child2) override;
};