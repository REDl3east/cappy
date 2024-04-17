include(FetchContent)

set(SDL_SHARED             OFF CACHE BOOL "" FORCE)
set(SDL_STATIC             ON  CACHE BOOL "" FORCE)
set(SDL_TEST               ON CACHE BOOL "" FORCE)
set(SDL_TESTS              ON CACHE BOOL "" FORCE)
set(SDL_INSTALL_TESTS      OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
  SDL3
  GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
  GIT_TAG 65a04a7  # main 04-16-24
  SOURCE_DIR ${SDL3_SRC_DIR}
)

set(BUILD_SHARED_LIBS                  OFF  CACHE BOOL "" FORCE)
set(CMAKE_POSITION_INDEPENDENT_CODE    ON  CACHE BOOL "" FORCE)
set(SDL3TTF_VENDORED                   ON  CACHE BOOL "" FORCE)
set(SDL3TTF_SAMPLES                    OFF  CACHE BOOL "" FORCE)
set(SDL3TTF_FREETYPE_VENDORED          ON  CACHE BOOL "" FORCE)

FetchContent_Declare(
  SDL3_ttf
  GIT_REPOSITORY https://github.com/libsdl-org/SDL_ttf
  GIT_TAG 23518cb  # main 04-16-24
  SOURCE_DIR ${SDL3_TTF_SRC_DIR}
)

set(NFD_BUILD_TESTS OFF  CACHE BOOL "" FORCE)
set(NFD_INSTALL     OFF  CACHE BOOL "" FORCE)

FetchContent_Declare(
  portable-file-dialogs
  GIT_REPOSITORY https://github.com/samhocevar/portable-file-dialogs
  GIT_TAG 7f852d8 # on Nov 11, 2022
  SOURCE_DIR ${DIALOG_SRC_DIR}
)

FetchContent_Declare(
  stb
  GIT_REPOSITORY https://github.com/nothings/stb
  GIT_TAG 5736b15   # main - 09-18-23
  SOURCE_DIR ${STB_SRC_DIR}
)

FetchContent_MakeAvailable(SDL3 SDL3_ttf portable-file-dialogs stb)