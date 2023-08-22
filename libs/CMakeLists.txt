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

FetchContent_MakeAvailable(SDL3)