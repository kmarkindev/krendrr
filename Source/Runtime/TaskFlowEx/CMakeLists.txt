add_krendrr_target(STATIC TaskFlowEx TARGET_NAME)

target_link_libraries(${TARGET_NAME} PUBLIC
    krendrr::thirdparty::taskflow
)

target_link_libraries(${TARGET_NAME} PRIVATE
    krendrr::thirdparty::nvtx
)