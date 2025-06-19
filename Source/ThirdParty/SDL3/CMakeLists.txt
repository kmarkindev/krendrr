FetchContent_Declare(
    sdl3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG c9a6709bd21750f1ad9597be21abace78c6378c9 # 3.2.16
    GIT_PROGRESS TRUE
)

set(SDL_STATIC ON)
set(SDL_SHARED OFF)
FetchContent_MakeAvailable(sdl3)

add_library(krendrr_thirdparty_sdl3 INTERFACE)
add_library(krendrr::thirdparty::sdl3 ALIAS krendrr_thirdparty_sdl3)

target_link_libraries(krendrr_thirdparty_sdl3 INTERFACE SDL3-static)