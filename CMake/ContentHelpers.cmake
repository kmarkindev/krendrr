
function(target_setup_content_folder TARGET CONTENT_FOLDER)

    set(CUSTOM_TARGET_NAME "copy_content_folder_${TARGET}")
    set(DESTINATION_FOLDER $<TARGET_FILE_DIR:${TARGET}>/Content)

    add_custom_target(
        ${CUSTOM_TARGET_NAME}
        COMMENT
            "Updating content folder for ${TARGET}"
        COMMAND
            ${CMAKE_COMMAND} -E make_directory ${DESTINATION_FOLDER}
        COMMAND
            # TODO: use rsync for UNIX systems and robocopy on WIN32 to mirror folders (to also remove files, not only copy)
            # robocopy ${CONTENT_FOLDER} ${DESTINATION_FOLDER} /MIR
            ${CMAKE_COMMAND} -E copy_directory_if_different ${CONTENT_FOLDER} ${DESTINATION_FOLDER}
    )

    add_dependencies(${TARGET} ${CUSTOM_TARGET_NAME})

endfunction()