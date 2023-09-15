include(FetchContent)

set(SDL2_DISABLE_SDL2MAIN  ON  CACHE BOOL "" FORCE)
set(SDL2_DISABLE_INSTALL   ON  CACHE BOOL "" FORCE)
set(SDL2_DISABLE_UNINSTALL ON  CACHE BOOL "" FORCE)
set(SDL_SHARED             OFF CACHE BOOL "" FORCE)
set(SDL_STATIC             ON  CACHE BOOL "" FORCE)
set(SDL_TEST               OFF CACHE BOOL "" FORCE)
set(SDL_TESTS              OFF CACHE BOOL "" FORCE)
set(SDL_INSTALL_TESTS      OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
  SDL3
  GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
  GIT_TAG 75d4a91  # main 07-17-23
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
  GIT_TAG 4a61384  # main 08-29-23
  SOURCE_DIR ${SDL3_TTF_SRC_DIR}
)

set(NFD_BUILD_TESTS OFF  CACHE BOOL "" FORCE)
set(NFD_INSTALL     OFF  CACHE BOOL "" FORCE)

FetchContent_Declare(
  nativefiledialog_extended
  GIT_REPOSITORY https://github.com/btzy/nativefiledialog-extended
  GIT_TAG 800f582   # v1.1.0
  SOURCE_DIR ${NFD_SRC_DIR}
)

FetchContent_MakeAvailable(SDL3 SDL3_ttf nativefiledialog_extended)