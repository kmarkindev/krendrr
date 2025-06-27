add_library(krendrr_common_utils STATIC)
add_library(krendrr::common::utils ALIAS krendrr_common_utils)
target_all_sources(krendrr_common_utils)

target_include_directories(krendrr_common_utils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Include)