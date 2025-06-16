FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 0af55ccecd98d4e5a8d1fad7de25ba429d60e863 # 1.0.1
    GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(glm)

add_library(krendrr_glm INTERFACE)
add_library(krendrr::glm ALIAS krendrr_glm)

target_link_libraries(krendrr_glm INTERFACE glm::glm)