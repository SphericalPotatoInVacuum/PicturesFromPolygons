#include <Selection.hpp>
#include <Chromosome.hpp>
#include <Utils.hpp>
#include <vector>
#include <stdexcept>

const char *selection_type_names[4] = {"Fitness Proportionate Selection", "Stochastic Universal Sampling",
                                       "Tournament Selection", "Truncation Selection"};

SelectionStrategy::SelectionStrategy(float cleansing_rate) : cleansing_rate_(cleansing_rate) {}

std::vector<Chromosome> FitnessPropotionateSelection::operator()(const std::vector<Chromosome> &chromosomes) {
  size_t size = chromosomes.size();
  std::vector<float> normalized_fitness(size);
  std::vector<float> acc_fitness(size);
  float total_fitness = 0;
  for (size_t i = 0; i < size; ++i) {
    total_fitness += chromosomes[i].GetFitness();
    acc_fitness[i] = total_fitness;
  }
  for (auto &fitness : acc_fitness) {
    fitness /= total_fitness;
  }
  size_t keep = size * (1 - cleansing_rate_);
  std::vector<Chromosome> new_chromosomes;
  for (size_t i = 0; i < keep; ++i) {
    float r = rand_float();
    size_t j = 0;
    while (r > acc_fitness[j]) {
      ++j;
    }
    new_chromosomes.emplace_back(chromosomes[j]);
  }
  return std::move(new_chromosomes);
}

std::vector<Chromosome> StochasticUniversalSampling::operator()(const std::vector<Chromosome> &chromosomes) {
  throw std::logic_error("Function not implemented");
}

std::vector<Chromosome> TournamentSelection::operator()(const std::vector<Chromosome> &chromosomes) {
  throw std::logic_error("Function not implemented");
}

std::vector<Chromosome> TruncationSelection::operator()(const std::vector<Chromosome> &chromosomes) {
  std::vector<Chromosome> new_chromosomes(chromosomes.begin(), chromosomes.end());
  std::sort(new_chromosomes.begin(), new_chromosomes.end(),
            [](const Chromosome &a, const Chromosome &b) { return a.GetFitness() > b.GetFitness(); });
  size_t keep = chromosomes.size() * (1 - cleansing_rate_);
  new_chromosomes.resize(keep);
  return std::move(new_chromosomes);
}