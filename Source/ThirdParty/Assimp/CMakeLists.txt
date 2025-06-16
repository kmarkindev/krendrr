FetchContent_Declare(
    assimp
    GIT_REPOSITORY https://github.com/assimp/assimp.git
    GIT_TAG fb375dd8c0a032106a2122815fb18dffe0283721 # 6.0.2
    GIT_PROGRESS TRUE
)

set(BUILD_SHARED_LIBS OFF)
set(ASSIMP_BUILD_TESTS OFF)
set(ASSIMP_COVERALLS OFF)
FetchContent_MakeAvailable(assimp)

add_library(krendrr_assimp INTERFACE)
add_library(krendrr::assimp ALIAS krendrr_assimp)

target_link_libraries(krendrr_assimp INTERFACE assimp)