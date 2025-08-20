
# Generates custom command to copy content folder for specified target
function(target_setup_content_folder TARGET)

    set(CUSTOM_TARGET_NAME "krendrr_copy_content_${TARGET}")
    set(DESTINATION_FOLDER ${CMAKE_BINARY_DIR}/Build/Content/${TARGET})

    add_custom_target(
        ${CUSTOM_TARGET_NAME}
        COMMENT
            "Updating content folder for ${TARGET}"
        COMMAND
            ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_SOURCE_DIR}/Content
        COMMAND
            ${CMAKE_COMMAND} -E make_directory ${DESTINATION_FOLDER}
        COMMAND
            # TODO: use rsync for UNIX systems and robocopy on WIN32 to mirror folders (to also remove files, not only copy)
            # robocopy ${CONTENT_FOLDER} ${DESTINATION_FOLDER} /MIR
            ${CMAKE_COMMAND} -E copy_directory_if_different ${CMAKE_CURRENT_SOURCE_DIR}/Content ${DESTINATION_FOLDER}
    )

    add_dependencies(${TARGET} ${CUSTOM_TARGET_NAME})

endfunction()