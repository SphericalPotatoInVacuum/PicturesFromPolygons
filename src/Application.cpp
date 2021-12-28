#include <cmath>
#include <cstdint>
#include <cstring>
#include <memory>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <glog/logging.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <imfilebrowser.h>

#include <Application.hpp>
#include <Utils.hpp>
#include <cstdlib>
#include <filesystem>

Application::Application(char *argv0) {
  InitLogger(argv0);
  InitGlfw();
  InitGlad();
  InitImgui();
  SetupOpenGL();
}

void Application::Run() {
  ImGui::FileBrowser file_dialog;
  std::filesystem::path input_path;
  std::string name_str = "";
  int triangles = 1000;
  int schedule = LINEAR;
  bool run = false;

  while (!glfwWindowShouldClose(window_)) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    /* Setup controls */
    {
      /* Window definition */
      ImGui::Begin("Control your setup", NULL, ImGuiWindowFlags_AlwaysAutoResize);
      ImGui::PushItemWidth(128.0f);

      if (ImGui::Button("Select file", ImVec2(128.0f, ImGui::GetItemRectSize().y))) {
        file_dialog.Open();
      }
      ImGui::SameLine();
      ImGui::Text("Selected file: %s", name_str.empty() ? "None" : name_str.c_str());

      if (ImGui::InputInt("Number of triangles", &triangles)) {
        if (triangles < 0) {
          triangles = 0;
        }
      }

      ImGui::Combo("Cooling schedule", &schedule, schedule_names, IM_ARRAYSIZE(schedule_names));

      if (ImGui::Button("START")) {
        if (name_str.empty()) {
          ImGui::OpenPopup("Select a file first");
        } else {
          triangle_count_ = static_cast<size_t>(triangles);
          Schedule_ = GetScheduleFunc(CoolingSchedule(schedule));
          StartAnnealing();
        }
      }
      if (ImGui::BeginPopupModal("Select a file first", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("You have not selected a file,\nplease select a target image first");
        if (ImGui::Button("OK")) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
      }

      ImGui::End();

      /* File dialog work */
      file_dialog.Display();

      if (file_dialog.HasSelected()) {
        annealing_ = false;
        input_path = file_dialog.GetSelected();
        GLubyte *image_data;
        if (LoadTextureFromFile(input_path.c_str(), &image_texture_, &image_width_, &image_height_, &image_data)) {
          /* If loading was a success then load this file */
          image_pixels_ = std::unique_ptr<GLubyte[]>(image_data);
          SetupNewTexture();
          name_str = input_path.filename().string();
          LOG(INFO) << "Selected file " << name_str;
        } else {
          /* Otherwise do nothing and tell the user to select another file */
          ImGui::OpenPopup("Wrong filetype");
        }
        file_dialog.ClearSelected();
      }

      if (ImGui::BeginPopupModal("Select a file first", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("You have not selected a file,\nplease select a target image first");
        if (ImGui::Button("OK")) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
      }
    }

    if (name_str != "") {
      /* If we have an active target file then draw it */
      ImGui::Begin("Target image", NULL, ImGuiWindowFlags_AlwaysAutoResize);
      ImGui::Image((void *)(intptr_t)image_texture_, ImVec2(image_width_, image_height_));
      ImGui::Text("Size = %d x %d", image_width_, image_height_);
      ImGui::End();
    }

    if (annealing_) {
      /* Annealing happens here */
      Mutate();

      DrawTriangles(cur_triangles_, cur_buffer_name_, cur_pixels_);
      MSE();

      float temp = Schedule_(iteration_);
      bool accept = cur_mse_ < best_mse_;
      if (!accept) {
        accept = rand_float() < std::exp((best_mse_ - cur_mse_) / temp);
      }

      if (accept) {
        best_triangles_ = cur_triangles_;
        best_mse_ = cur_mse_;
        DrawTriangles(best_triangles_, best_buffer_name_, best_pixels_);
      }

      temps_.push_back(temp);
      mses_.push_back(best_mse_);

      ImGui::Begin("Evaluating image", NULL, ImGuiWindowFlags_AlwaysAutoResize);
      ImGui::Image((void *)(intptr_t)cur_texture_, ImVec2(image_width_, image_height_));
      ImGui::End();

      ImGui::Begin("Current image", NULL, ImGuiWindowFlags_AlwaysAutoResize);
      ImGui::Image((void *)(intptr_t)best_texture_, ImVec2(image_width_, image_height_));
      ImGui::End();

      ImGui::Begin("Stats", NULL, ImGuiWindowFlags_AlwaysAutoResize);
      ImGui::Text("Current iteration: %lu", iteration_);
      ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
                  ImGui::GetIO().Framerate);
      ImGui::PlotLines("Temp", temps_.data(), temps_.size());
      ImGui::SameLine();
      ImGui::Text("%.5f", temp);
      ImGui::PlotLines("MSE", mses_.data(), mses_.size());
      ImGui::SameLine();
      ImGui::Text("%.2f", best_mse_);

      ImGui::End();

      iteration_++;
    }

    ImGui::Render();

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glFlush();

    glfwSwapBuffers(window_);
    glfwPollEvents();
  }

  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  GlfwTeardown();
}

void Application::InitLogger(char *argv0) {
  FLAGS_logtostderr = 1;
  FLAGS_log_dir = "logs/";
  google::InitGoogleLogging(argv0);
  LOG(INFO) << "Initialized glog";
}

void Application::InitGlfw() {
  if (!glfwInit()) {
    LOG(ERROR) << "Could not initialize glfw";
    exit(2);
  }
  LOG(INFO) << "Initialized glfw";

  glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_FALSE);
  window_ = glfwCreateWindow(kWidth, kHeight, "PFP", NULL, NULL);
  if (window_ == NULL) {
    LOG(ERROR) << "Could not create GLFW window";
    GlfwTeardown();
    exit(3);
  }
  glfwMakeContextCurrent(window_);
  glfwSwapInterval(0);
  LOG(INFO) << "Created GLFW window";
}

void Application::InitGlad() {
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    LOG(ERROR) << "Could not initialize glad";
    GlfwTeardown();
    exit(4);
  }
  LOG(INFO) << "Initialized glad";
}

void Application::InitImgui() {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  (void)io;

  ImGui::StyleColorsDark();

  ImGui_ImplGlfw_InitForOpenGL(window_, true);
  ImGui_ImplOpenGL3_Init("#version 150");
}

void Application::SetupOpenGL() {
  glfwGetWindowSize(window_, &actual_width_, &actual_height_);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glViewport(0, 0, actual_width_, actual_height_);
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  LOG(INFO) << "Initialized OpenGL";
}

void Application::GlfwTeardown() {
  if (window_ != NULL) {
    glfwDestroyWindow(window_);
  }
  glfwTerminate();

  LOG(INFO) << "Teardown finished";
}

/* Create new OpenGL textures for generated images */
void Application::SetupNewTexture() {
  GLuint buffer_names[2] = {0};
  GLuint textures[2] = {0};
  glGenFramebuffers(2, buffer_names);
  glGenTextures(2, textures);

  cur_buffer_name_ = buffer_names[0];
  best_buffer_name_ = buffer_names[1];

  cur_texture_ = textures[0];
  best_texture_ = textures[1];

  glBindFramebuffer(GL_FRAMEBUFFER, cur_buffer_name_);
  glBindTexture(GL_TEXTURE_2D, cur_texture_);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width_, image_height_, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, cur_texture_, 0);

  glBindFramebuffer(GL_FRAMEBUFFER, best_buffer_name_);
  glBindTexture(GL_TEXTURE_2D, best_texture_);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width_, image_height_, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, best_texture_, 0);

  GLenum buffers[2] = {GL_COLOR_ATTACHMENT0};
  glDrawBuffers(2, buffers);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Application::Mutate() {
  MutationType mutation = MutationType(rand() % MutationType::LAST);
  cur_triangles_ = best_triangles_;
  switch (mutation) {
    case COLOR: {
      int idx = rand() % cur_triangles_.size();
      Triangle &tr = cur_triangles_[idx];
      idx = rand() % 4;
      tr.color[idx] = clamp(tr.color[idx] + rand_float(-0.1f, +0.1f), 0.0f, 1.0f);
      break;
    }
    case ORDER: {
      assert(triangles_.size() > 1);
      int idx1 = rand() % cur_triangles_.size();
      int idx2 = idx1;
      while (idx2 == idx1) {
        idx1 = rand() % cur_triangles_.size();
      }
      std::swap(cur_triangles_[idx1], cur_triangles_[idx2]);
      break;
    }
    case POSITION: {
      int idx = rand() % cur_triangles_.size();
      Triangle &tr = cur_triangles_[idx];
      idx = rand() % 3;
      tr.vs[idx].x = clamp(tr.vs[idx].x + rand_float(-0.1f, +0.1f), -1.0f, 1.0f);
      tr.vs[idx].y = clamp(tr.vs[idx].y + rand_float(-0.1f, +0.1f), -1.0f, 1.0f);
      break;
    }
  }
}

void Application::DrawTriangles(const std::vector<Triangle> &triangles, GLuint buffer,
                                std::unique_ptr<GLubyte[]> &pixels) {
  glBindFramebuffer(GL_FRAMEBUFFER, buffer);
  glViewport(0, 0, image_width_, image_height_);
  glClear(GL_COLOR_BUFFER_BIT);
  glBegin(GL_TRIANGLES);
  for (const auto &tr : triangles) {
    glColor4f(tr.color[0], tr.color[1], tr.color[2], tr.color[3]);
    for (int i = 0; i < 3; ++i) {
      glVertex2f(tr.vs[i].x, tr.vs[i].y);
    }
  }
  glEnd();
  glReadPixels(0, 0, image_width_, image_height_, GL_RGBA, GL_UNSIGNED_BYTE, pixels.get());
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Application::StartAnnealing() {
  best_triangles_.resize(triangle_count_);
  best_pixels_ = std::unique_ptr<GLubyte[]>((GLubyte *)std::malloc(4 * image_width_ * image_height_));
  for (auto &tr : best_triangles_) {
    tr = {{{rand_float(-1, 1), rand_float(-1, 1)},
           {rand_float(-1, 1), rand_float(-1, 1)},
           {rand_float(-1, 1), rand_float(-1, 1)}},
          {rand_float(), rand_float(), rand_float(), rand_float()}};
  }
  DrawTriangles(best_triangles_, best_buffer_name_, best_pixels_);
  cur_triangles_ = best_triangles_;
  cur_pixels_ = std::unique_ptr<GLubyte[]>((GLubyte *)std::malloc(4 * image_width_ * image_height_));
  std::memcpy(cur_pixels_.get(), best_pixels_.get(), 4 * image_width_ * image_height_);
  MSE();
  best_mse_ = cur_mse_;
  annealing_ = true;
  iteration_ = 1;
  mses_.clear();
  temps_.clear();
  LOG(INFO) << "Started simulation, initial MSE: " << cur_mse_;
}

ScheduleFunc Application::GetScheduleFunc(CoolingSchedule schedule) {
  switch (schedule) {
    case CoolingSchedule::C_1:
      return [](int i) { return 1.0f / std::log(i + 1); };
    case CoolingSchedule::C_50:
      return [](int i) { return 50.0f / std::log(i + 1); };
    case CoolingSchedule::C_195075:
      return [](int i) { return 195075.0f / std::log(i + 1); };
    default:
      return [](int i) { return 1.0f / i; };
  }
}

void Application::MSE() {
  uint64_t mse = 0;
  int diff = 0;
  for (size_t i = 0; i < 4 * image_width_ * image_height_; ++i) {
    diff = cur_pixels_[i] - image_pixels_[i];
    mse += diff * diff;
  }
  cur_mse_ = static_cast<double>(mse) / (4.0 * image_width_ * image_height_);
}
