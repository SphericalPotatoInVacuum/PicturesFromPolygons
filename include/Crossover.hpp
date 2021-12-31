#pragma once

#include <Chromosome.hpp>

enum CrossoverType { ONE_POINT, TWO_POINT, UNIFORM };

extern const char *crossover_type_names[3];

class CrossoverStrategy {
 public:
  virtual Chromosome operator()(const Chromosome &child1, const Chromosome &child2);
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