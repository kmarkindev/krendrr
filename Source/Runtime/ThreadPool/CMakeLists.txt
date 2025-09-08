add_krendrr_target(STATIC ThreadPool TARGET_NAME)

target_link_libraries(${TARGET_NAME} PRIVATE
    krendrr::thirdparty::nvtx
)