cmake_minimum_required(VERSION 3.27.1...3.27.1)
project(cappy VERSION 0.1.0 LANGUAGES C)


set(SDL3_SRC_DIR     ${CMAKE_CURRENT_SOURCE_DIR}/libs/sdl3)
set(SDL3_TTF_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libs/sdl3_ttf)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/libs)

set(MAIN_SRC
  ${CMAKE_CURRENT_SOURCE_DIR}/src/main.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/sdl_callback.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/capture.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/config.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/camera.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/renderer.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/stb.c
  
  ${CMAKE_CURRENT_SOURCE_DIR}/src/state/state_move.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/state/state_color.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/state/state_flashlight.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/state/state_draw.c
)

add_executable(cappy ${MAIN_SRC})
target_include_directories(cappy PRIVATE 
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${CMAKE_CURRENT_SOURCE_DIR}/src/state
  ${CMAKE_CURRENT_SOURCE_DIR}/libs
)

if (UNIX)
    target_link_libraries(cappy SDL3-static SDL3_ttf-static X11)
elseif(WIN32)
    set_property(TARGET cappy PROPERTY WIN32_EXECUTABLE true)
    target_link_libraries(cappy SDL3-static SDL3_ttf-static)
endif()
    
install(TARGETS cappy DESTINATION bin)