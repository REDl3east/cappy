cmake_minimum_required(VERSION 3.0.0)
project(cappy VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)

set(SDL3_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libs/sdl3)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/libs)

set(MAIN_SRC
  ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/camera.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/capture.cpp
)

add_executable(cappy ${MAIN_SRC})
target_link_libraries(cappy SDL3-static X11)