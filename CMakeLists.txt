cmake_minimum_required(VERSION 3.11)

set(CMAKE_CXX_STANDARD 20)

project(j CXX)

set(IMGUI_ROOT imgui)

file(GLOB IMGUI_SRC ${IMGUI_ROOT}/*.cpp ${IMGUI_ROOT}/*.h)
set(IMGUI_BACKEND_SRC ${IMGUI_ROOT}/backends/imgui_impl_glfw.cpp ${IMGUI_ROOT}/backends/imgui_impl_opengl3.cpp)

find_package(OpenGL REQUIRED)
find_library(COCOA_LIBRARY Cocoa)
find_library(IOKIT_LIBRARY IOKit)

add_executable(main main.cpp ${IMGUI_SRC} ${IMGUI_BACKEND_SRC})

add_subdirectory(glfw)

target_link_directories(main PRIVATE /usr/local/lib)
target_include_directories(main PRIVATE /usr/local/include)

target_link_directories(main PRIVATE ${IMGUI_ROOT})
target_include_directories(main PRIVATE ${IMGUI_ROOT})

target_link_directories(main PRIVATE ${IMGUI_ROOT}/backends)
target_include_directories(main PRIVATE ${IMGUI_ROOT}/backends)

target_link_libraries(main ${OPENGL_LIBRARIES} glfw3 ${COCOA_LIBRARY} ${IOKIT_LIBRARY})
