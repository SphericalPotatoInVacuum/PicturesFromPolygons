cmake_minimum_required(VERSION 3.16)
project(PaintingsFromPolygons VERSION 0.0.1
                        DESCRIPTION  "Paintings from polygons"
                        LANGUAGES CXX C)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIRS})

add_library(glad
    libs/glad/src/glad.c
)
target_include_directories(glad PUBLIC libs/glad/include)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "GLFW lib only")
set(GLFW_INSTALL OFF CACHE BOOL "GLFW lib only")

add_subdirectory(libs/glfw)
include_directories(libs/glfw/include)

add_library(imgui
    libs/imgui/imgui.cpp libs/imgui/imgui_draw.cpp
    libs/imgui/imgui_tables.cpp libs/imgui/imgui_widgets.cpp
    libs/imgui/imgui_demo.cpp
    libs/imgui/backends/imgui_impl_glfw.cpp
    libs/imgui/backends/imgui_impl_opengl3.cpp
)
target_include_directories(imgui PUBLIC libs/imgui libs/imgui/backends)
target_compile_definitions(imgui PUBLIC GL_GLEXT_PROTOTYPES=1)
target_link_libraries(imgui PUBLIC glfw)

add_executable(app src/main.cpp src/Application.cpp src/Utils.cpp)
target_include_directories(app PUBLIC include)
target_include_directories(app PUBLIC libs/imgui-filebrowser)
target_compile_features(app PUBLIC cxx_std_17)
target_link_libraries(app PUBLIC glad glfw imgui ${OPENGL_LIBRARIES} ${CMAKE_DL_LIBS})
