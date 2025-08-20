# convert krendrr/Source/dir1/dir2/my_target to dir1_dir2_my_target and dir1::dir2::my_target
function(make_target_name_from_path UNDERSCORE_RESULT_VAR DOTTED_RESULT_VAR TARGET_FOLDER_PATH)

    file(RELATIVE_PATH TARGET_FOLDER_RELATIVE_PATH ${CMAKE_SOURCE_DIR}/Source ${TARGET_FOLDER_PATH})

    # convert relative folder path
    string(REPLACE "/" "_" UNDERSCORE_PATH_RESULT ${TARGET_FOLDER_RELATIVE_PATH})
    string(REPLACE "/" "::" DOTTED_PATH_RESULT ${TARGET_FOLDER_RELATIVE_PATH})
    string(TOLOWER ${UNDERSCORE_PATH_RESULT} UNDERSCORE_PATH_RESULT)
    string(TOLOWER ${DOTTED_PATH_RESULT} DOTTED_PATH_RESULT)

    string(CONCAT UNDERSCORE_PATH_RESULT "krendrr_" ${UNDERSCORE_PATH_RESULT})
    string(CONCAT DOTTED_PATH_RESULT "krendrr::" ${DOTTED_PATH_RESULT})

    set(${UNDERSCORE_RESULT_VAR} ${UNDERSCORE_PATH_RESULT} PARENT_SCOPE)
    set(${DOTTED_RESULT_VAR} ${DOTTED_PATH_RESULT} PARENT_SCOPE)

endfunction()

# determine krendrr target category
function(get_krendrr_target_category RESULT_VAR TARGET_FOLDER_PATH)

    cmake_path(GET TARGET_FOLDER_PATH PARENT_PATH PARENTED_PATH)

    file(RELATIVE_PATH TARGET_FOLDER_RELATIVE_PATH ${CMAKE_SOURCE_DIR}/Source ${PARENTED_PATH})
    string(TOLOWER ${PARENTED_PATH} PARENTED_PATH)

    string(FIND ${PARENTED_PATH} "program" PROGRAM_POS)
    string(FIND ${PARENTED_PATH} "runtime" RUNTIME_POS)
    string(FIND ${PARENTED_PATH} "example" EXAMPLE_POS)
    string(FIND ${PARENTED_PATH} "thirdparty" THIRDPARTY_POS)

    if(PROGRAM_POS GREATER_EQUAL 0)
        set(${RESULT_VAR} "Program" PARENT_SCOPE)
    elseif (RUNTIME_POS GREATER_EQUAL 0)
        set(${RESULT_VAR} "Runtime" PARENT_SCOPE)
    elseif (EXAMPLE_POS GREATER_EQUAL 0)
        set(${RESULT_VAR} "Example" PARENT_SCOPE)
    elseif (THIRDPARTY_POS GREATER_EQUAL 0)
        set(${RESULT_VAR} "ThirdParty" PARENT_SCOPE)
    else ()
        message(FATAL_ERROR "Could not detect krendrr target category (example, runtime, program or thirdparty)")
    endif ()

endfunction()

# Creates new target, sets up content folder and sources, generates correct prefix
function(add_krendrr_target TARGET_TYPE TARGET_OUTPUT_NAME OUT_TARGET_NAME)

    make_target_name_from_path(UNDERSCORE_TARGET_NAME DOTTED_TARGET_NAME ${CMAKE_CURRENT_SOURCE_DIR})

    get_krendrr_target_category(TARGET_CATEGORY ${CMAKE_CURRENT_SOURCE_DIR})

    if(TARGET_TYPE STREQUAL EXECUTABLE)
        add_executable(${UNDERSCORE_TARGET_NAME})
    elseif (TARGET_TYPE STREQUAL STATIC)
        add_library(${UNDERSCORE_TARGET_NAME} STATIC)
    elseif (TARGET_TYPE STREQUAL SHARED)
        add_library(${UNDERSCORE_TARGET_NAME} SHARED)
    elseif (TARGET_TYPE STREQUAL INTERFACE)
        add_library(${UNDERSCORE_TARGET_NAME} INTERFACE)
    elseif ()
        message(FATAL_ERROR "Unsupported target type provided when creating krendrr target")
    endif ()

    set_target_properties(${UNDERSCORE_TARGET_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Build/${TARGET_CATEGORY}
        RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/Build/${TARGET_CATEGORY}
        RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/Build/${TARGET_CATEGORY}
        RUNTIME_OUTPUT_NAME ${TARGET_OUTPUT_NAME}
    )

    set(${OUT_TARGET_NAME} ${UNDERSCORE_TARGET_NAME} PARENT_SCOPE)

    if(NOT TARGET_TYPE STREQUAL EXECUTABLE)
        add_library(${DOTTED_TARGET_NAME} ALIAS ${UNDERSCORE_TARGET_NAME})
    endif ()

    target_all_sources(${UNDERSCORE_TARGET_NAME})

    target_include_directories(${UNDERSCORE_TARGET_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Include)

    target_setup_content_folder(${UNDERSCORE_TARGET_NAME})

endfunction()