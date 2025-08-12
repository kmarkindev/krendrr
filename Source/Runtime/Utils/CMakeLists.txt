add_library(krendrr_runtime_utils STATIC)
add_library(krendrr::runtime::utils ALIAS krendrr_runtime_utils)
target_all_sources(krendrr_runtime_utils)

target_include_directories(krendrr_runtime_utils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Include)