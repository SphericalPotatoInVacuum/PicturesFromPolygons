cmake_minimum_required(VERSION 3.16)

project(PaintingsFromPolygons VERSION 0.0.1
                        DESCRIPTION  "Paintings from polygons"
                        LANGUAGES CXX C)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -std=c++17")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_DEBUG} -g")
set(CMAKE_CXX_FLAGS_RELEASEE "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELEASE} -O3")

add_subdirectory(libs/glog)

find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIRS})

add_library(glad
    libs/glad/src/glad.c
)
target_include_directories(glad PUBLIC libs/glad/include)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "GLFW lib only")
set(GLFW_INSTALL OFF CACHE BOOL "GLFW lib only")

add_subdirectory(libs/glfw)

add_library(imgui
    libs/imgui/imgui.cpp libs/imgui/imgui_draw.cpp
    libs/imgui/imgui_tables.cpp libs/imgui/imgui_widgets.cpp
    libs/imgui/imgui_demo.cpp
    libs/imgui/backends/imgui_impl_glfw.cpp
    libs/imgui/backends/imgui_impl_opengl3.cpp
)
target_include_directories(imgui PUBLIC libs/imgui libs/imgui/backends)
target_compile_definitions(imgui PUBLIC GL_GLEXT_PROTOTYPES=1)

add_executable(app src/main.cpp src/Application.cpp src/Utils.cpp)
target_include_directories(app PUBLIC include)
target_include_directories(app PUBLIC libs/imgui-filebrowser)
target_link_libraries(app PUBLIC glad glfw glog::glog imgui ${OPENGL_LIBRARIES} ${CMAKE_DL_LIBS})
