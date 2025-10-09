include(FetchContent)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

set(SDL_SHARED             OFF CACHE BOOL "" FORCE)
set(SDL_STATIC             ON  CACHE BOOL "" FORCE)
set(SDL_TEST               OFF CACHE BOOL "" FORCE)
set(SDL_TESTS              OFF CACHE BOOL "" FORCE)
set(SDL_INSTALL_TESTS      OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
  SDL3
  GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
  GIT_TAG release-3.2.18
  SOURCE_DIR ${SDL3_SRC_DIR}
)

set(BUILD_SHARED_LIBS                  OFF  CACHE BOOL "" FORCE)
set(CMAKE_POSITION_INDEPENDENT_CODE    ON   CACHE BOOL "" FORCE)
set(SDL3TTF_VENDORED                   ON   CACHE BOOL "" FORCE)
set(SDL3TTF_SAMPLES                    OFF  CACHE BOOL "" FORCE)
set(SDL3TTF_FREETYPE_VENDORED          ON   CACHE BOOL "" FORCE)

FetchContent_Declare(
  SDL3_ttf
  GIT_REPOSITORY https://github.com/libsdl-org/SDL_ttf
  GIT_TAG release-3.2.2
  SOURCE_DIR ${SDL3_TTF_SRC_DIR}
)

FetchContent_MakeAvailable(SDL3 SDL3_ttf)