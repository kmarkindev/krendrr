FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG 55f93686c01528224f448c19128836e7df245f72 # 3.12.0
)
FetchContent_MakeAvailable(nlohmann_json)

add_library(krendrr_thirdparty_nlohmann_json INTERFACE)
add_library(krendrr::thirdparty::nlohmann_json ALIAS krendrr_thirdparty_nlohmann_json)

target_include_directories(krendrr_thirdparty_nlohmann_json INTERFACE ${nlohmann_json_SOURCE_DIR}/single_include)