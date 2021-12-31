#include <Chromosome.hpp>
#include <Utils.hpp>
#include <cassert>

Chromosome::Chromosome(const size_t size) : triangles_(size) {
  for (auto &tr : triangles_) {
    tr = {{{rand_float(-1, 1), rand_float(-1, 1)},
           {rand_float(-1, 1), rand_float(-1, 1)},
           {rand_float(-1, 1), rand_float(-1, 1)}},
          {rand_float(), rand_float(), rand_float(), rand_float()}};
  }
}

Chromosome::Chromosome(std::vector<Triangle> triangles) : triangles_(triangles){};

void Chromosome::Mutate() {
  MutationType mutation = MutationType(rand() % MutationType::LAST);
  switch (mutation) {
    case COLOR: {
      int idx = rand() % triangles_.size();
      Triangle &tr = triangles_[idx];
      idx = rand() % 4;
      tr.color[idx] = clamp(tr.color[idx] + rand_float(-0.1f, +0.1f), 0.0f, 1.0f);
      break;
    }
    case ORDER: {
      assert(triangles_.size() > 1);
      int idx1 = rand() % triangles_.size();
      int idx2 = idx1;
      while (idx2 == idx1) {
        idx1 = rand() % triangles_.size();
      }
      std::swap(triangles_[idx1], triangles_[idx2]);
      break;
    }
    case POSITION: {
      int idx = rand() % triangles_.size();
      Triangle &tr = triangles_[idx];
      idx = rand() % 3;
      tr.vs[idx].x = clamp(tr.vs[idx].x + rand_float(-0.1f, +0.1f), -1.0f, 1.0f);
      tr.vs[idx].y = clamp(tr.vs[idx].y + rand_float(-0.1f, +0.1f), -1.0f, 1.0f);
      break;
    }
  }
}

void Chromosome::Draw(GLuint buffer, int image_width, int image_height) {
  glBindFramebuffer(GL_FRAMEBUFFER, buffer);
  glViewport(0, 0, image_width, image_height);
  glClear(GL_COLOR_BUFFER_BIT);
  glBegin(GL_TRIANGLES);
  for (const auto &tr : triangles_) {
    glColor4f(tr.color.r, tr.color.g, tr.color.b, tr.color.a);
    for (int i = 0; i < 3; ++i) {
      glVertex2f(tr.vs[i].x, tr.vs[i].y);
    }
  }
  glEnd();
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

const std::vector<Triangle> &Chromosome::GetTriangles() const {
  return triangles_;
}

void Chromosome::SetFitness(float fitness) {
  fitness_ = fitness;
}

float Chromosome::GetFitness() const {
  return fitness_;
}

const Triangle &Chromosome::operator[](size_t idx) const {
  return triangles_[idx];
}