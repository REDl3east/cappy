cmake_minimum_required(VERSION 3.0.0)
project(cappy VERSION 0.1.0 LANGUAGES C CXX)

set(MAIN_SRC
  ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/capture.cpp
)

add_executable(cappy ${MAIN_SRC})
target_link_libraries(cappy X11)