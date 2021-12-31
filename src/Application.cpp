#include <cmath>
#include <cstdint>
#include <cstring>
#include <memory>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <imfilebrowser.h>

#include <Application.hpp>
#include <Utils.hpp>
#include <cstdlib>
#include <filesystem>
#include <plog/Log.h>
#include <plog/Init.h>
#include <plog/Appenders/ConsoleAppender.h>
#include <plog/Appenders/RollingFileAppender.h>
#include <plog/Formatters/TxtFormatter.h>

Application::Application() {
  InitLogging();
  InitGlfw();
  InitGlad();
  InitImgui();
  SetupOpenGL();
}

void Application::Run() {
  ImGui::FileBrowser file_dialog;
  std::filesystem::path input_path;
  std::string filename_str = "";
  int population_size = 8;
  int genome_size = 200;
  float cleansing_rate = 0.75f;
  int crossover_type = CrossoverType::UNIFORM;
  int selection_type = SelectionType::FITNESS_PROPORTIONATE_SELECTION;

  bool flag = true;

  while (!glfwWindowShouldClose(window_)) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Setup controls
    {
      // Window definition
      ImGui::Begin("Control your setup", NULL, ImGuiWindowFlags_AlwaysAutoResize);
      ImGui::PushItemWidth(128.0f);

      if (ImGui::Button("Select file", ImVec2(128.0f, ImGui::GetItemRectSize().y))) {
        file_dialog.Open();
      }
      ImGui::SameLine();
      ImGui::Text("Selected file: %s", filename_str.empty() ? "None" : filename_str.c_str());

      ImGui::DragInt("Population size", &population_size, 1.0f, 2, 50, "%d", ImGuiSliderFlags_AlwaysClamp);

      ImGui::DragInt("Genome size", &genome_size, 1.0f, 1, 10000, "%d", ImGuiSliderFlags_AlwaysClamp);

      ImGui::DragFloat("Cleansing rate", &cleansing_rate, 0.01f, 0.0f, 1.0f, "%4.2f", ImGuiSliderFlags_AlwaysClamp);

      ImGui::Combo("Crossover type", &crossover_type, crossover_type_names, IM_ARRAYSIZE(crossover_type_names));

      ImGui::Combo("Selection type", &selection_type, selection_type_names, IM_ARRAYSIZE(selection_type_names));

      if (ImGui::Button("START")) {
        if (input_path.empty()) {
          ImGui::OpenPopup("Select a file first");
        } else {
          solver_.Cleanup();
          solver_ = Solver(image_, population_size, genome_size, cleansing_rate, CrossoverType(crossover_type),
                           SelectionType(selection_type));
          Start();
        }
      }

      if (ImGui::BeginPopupModal("Select a file first", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("You have not selected a file,\nplease select a target image first");
        if (ImGui::Button("OK")) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
      }

      ImGui::End();
    }

    // File dialog work
    {
      file_dialog.Display();

      if (file_dialog.HasSelected()) {
        running_ = false;
        input_path = file_dialog.GetSelected();
        if (LoadTextureFromFile(input_path, image_)) {
          // If loading was a success then load this file
          filename_str = input_path.filename().string();
        } else {
          // Otherwise do nothing and tell the user to select another file
          ImGui::OpenPopup("Wrong filetype");
        }
        file_dialog.ClearSelected();
      }
    }

    if (ImGui::BeginPopupModal("Select a file first", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
      ImGui::Text("You have not selected a file,\nplease select a target image first");
      if (ImGui::Button("OK")) ImGui::CloseCurrentPopup();
      ImGui::EndPopup();
    }

    if (!input_path.empty()) {
      // If we have an active target file then draw it
      ImGui::Begin("Target image", NULL, ImGuiWindowFlags_AlwaysAutoResize);
      ImGui::Image((void *)(intptr_t)image_.texture, ImVec2(image_.width, image_.height));
      ImGui::Text("Size = %d x %d", image_.width, image_.height);
      ImGui::End();
    }

    if (running_) {
      PLOGI << "Before iteration";
      IterationResult res = solver_.Iteration();
      PLOGI << "After iteration";
      ImGui::Begin("Current best", NULL, ImGuiWindowFlags_AlwaysAutoResize);
      ImGui::Image((void *)(intptr_t)res.texture, ImVec2(image_.width, image_.height));
      ImGui::End();

      ImGui::Begin("Stats", NULL, ImGuiWindowFlags_AlwaysAutoResize);
      ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
                  ImGui::GetIO().Framerate);
      ImGui::Text("Current iteration: %lu", res.iteration);
      ImGui::Text("Mean MSE: %.2f", res.mse_mean);
      ImGui::Text("Best MSE: %.2f", res.mse_best);
      ImGui::Text("Worst MSE: %.2f", res.mse_worst);
      ImGui::End();
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

void Application::InitLogging() {
  static plog::RollingFileAppender<plog::TxtFormatter> fileAppender("log.txt", 1000000, 3);
  static plog::ConsoleAppender<plog::TxtFormatter> consoleAppender;
  plog::init(plog::debug, &fileAppender).addAppender(&consoleAppender);
  PLOGI << "Init PLOG: OK";
}

void Application::InitGlfw() {
  if (!glfwInit()) {
    PLOGE << "Init GLFW: FAIL";
    exit(2);
  }
  PLOGI << "Init GLFW: OK";

  glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_FALSE);
  window_ = glfwCreateWindow(kWidth, kHeight, "PFP", NULL, NULL);
  if (window_ == NULL) {
    PLOGE << "GLFW window creation: FAIL";
    GlfwTeardown();
    exit(3);
  }
  glfwMakeContextCurrent(window_);
  glfwSwapInterval(0);
  PLOGI << "Setup GLFW: OK";
}

void Application::InitGlad() {
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    GlfwTeardown();
    PLOGE << "Init GLAD: FAIL";
    exit(4);
  }
  PLOGI << "Init GLAD: OK";
}

void Application::InitImgui() {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  (void)io;

  ImGui::StyleColorsDark();

  ImGui_ImplGlfw_InitForOpenGL(window_, true);
  ImGui_ImplOpenGL3_Init("#version 150");

  PLOGI << "Init Dear ImGUI: OK";
}

void Application::SetupOpenGL() {
  glfwGetWindowSize(window_, &actual_width_, &actual_height_);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glViewport(0, 0, actual_width_, actual_height_);
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

  PLOGI << "Setup OpenGL: OK";
}

void Application::GlfwTeardown() {
  if (window_ != NULL) {
    glfwDestroyWindow(window_);
  }
  glfwTerminate();

  PLOGI << "GLFW terminated";
}

void Application::Start() {
  running_ = true;
  PLOGI << "Started algorithm";
}