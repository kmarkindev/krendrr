FetchContent_Declare(
    minmalloc
    GIT_REPOSITORY https://github.com/microsoft/mimalloc.git
    GIT_TAG 3e0449177db77807e2d3f5b2b60fbcb87f286d8a # 3.0.9
    GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(minmalloc)

add_library(krendrr_thirdparty_minmalloc INTERFACE)
add_library(krendrr::thirdparty::minmalloc ALIAS krendrr_thirdparty_minmalloc)

target_link_libraries(krendrr_thirdparty_minmalloc INTERFACE mimalloc)