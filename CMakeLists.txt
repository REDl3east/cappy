cmake_minimum_required(VERSION 3.0.0)
project(cappy VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)

set(SDL3_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libs/sdl3)
set(SDL3_TTF_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libs/sdl3_ttf)
set(DIALOG_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libs/portable-file-dialogs)
set(STB_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libs/stb)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/libs)

set(MAIN_SRC
  ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/camera.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/capture.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/renderer.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/stb.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/machine/cappyMachine.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/state/colorState.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/state/drawCropState.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/state/flashlightState.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/state/moveState.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/config.cpp
)

add_executable(cappy ${MAIN_SRC})
target_include_directories(cappy PRIVATE 
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${CMAKE_CURRENT_SOURCE_DIR}/src/machine
  ${CMAKE_CURRENT_SOURCE_DIR}/src/state
  ${SDL3_SRC_DIR}/include
  ${SDL3_TTF_SRC_DIR}/include
  ${DIALOG_SRC_DIR}
  ${STB_SRC_DIR}
)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set_target_properties(cappy
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/debug/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/debug/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/debug/bin"
  )
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
  set_target_properties(cappy
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/release/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/release/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/release/bin"
  )
endif()

if (UNIX)
    target_link_libraries(cappy SDL3-static SDL3_ttf-static X11)
    elseif(WIN32)
    set_property(TARGET cappy PROPERTY WIN32_EXECUTABLE true)
    target_link_libraries(cappy SDL3-static SDL3_ttf-static)
endif()
    
install(TARGETS cappy DESTINATION bin)