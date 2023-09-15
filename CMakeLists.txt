cmake_minimum_required(VERSION 3.0.0)
project(cappy VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)

set(SDL3_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libs/sdl3)
set(SDL3_TTF_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libs/sdl3_ttf)
set(NFD_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libs/nativefiledialog-extended)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/libs)

set(MAIN_SRC
  ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/camera.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/capture.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/machine.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/renderer.cpp
)

add_executable(cappy ${MAIN_SRC})
target_include_directories(cappy PRIVATE 
  ${SDL3_SRC_DIR}/include
  ${SDL3_TTF_SRC_DIR}/include
)
if (UNIX)
    target_link_libraries(cappy nfd SDL3-static SDL3_ttf-static X11)
elseif(WIN32)
    target_link_libraries(cappy nfd SDL3-static SDL3_ttf-static)
endif()