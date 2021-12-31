#include <Crossover.hpp>
#include <Chromosome.hpp>
#include <cassert>

const char *crossover_type_names[3] = {"One Point", "Two Point", "Uniform"};

Chromosome OnePointCrossoverStrategy::operator()(const Chromosome &child1, const Chromosome &child2) {
  assert(child1.GetTriangles().size() == child2.GetTriangles().size());
  size_t size = child1.GetTriangles().size();
  size_t idx = rand() % (size - 1) + 1;
  std::vector<Triangle> triangles(size);
  for (size_t i = 0; i < idx; ++i) {
    triangles[i] = child1[i];
  }
  for (size_t i = idx; i < size; ++i) {
    triangles[i] = child2[i];
  }
  return Chromosome(std::move(triangles));
}

Chromosome TwoPointCrossoverStrategy::operator()(const Chromosome &child1, const Chromosome &child2) {
  assert(child1.GetTriangles().size() == child2.GetTriangles().size());
  size_t size = child1.GetTriangles().size();
  size_t idx1 = rand() % (size - 1) + 1;
  size_t idx2 = rand() % (size - 2) + 1;
  if (idx2 >= idx1) {
    idx2++;
  } else {
    std::swap(idx1, idx2);
  }
  std::vector<Triangle> triangles(size);
  for (size_t i = 0; i < idx1; ++i) {
    triangles[i] = child1[i];
  }
  for (size_t i = idx1; i < idx2; ++i) {
    triangles[i] = child2[i];
  }
  for (size_t i = idx2; i < size; ++i) {
    triangles[i] = child1[i];
  }
  return Chromosome(std::move(triangles));
}

Chromosome UniformCrossoverStrategy::operator()(const Chromosome &child1, const Chromosome &child2) {
  assert(child1.GetTriangles().size() == child2.GetTriangles().size());
  size_t size = child1.GetTriangles().size();
  std::vector<Triangle> triangles(size);
  for (size_t i = 0; i < size; ++i) {
    if (rand() % 2) {
      triangles[i] = child1[i];
    } else {
      triangles[i] = child2[i];
    }
  }
  return Chromosome(std::move(triangles));
}