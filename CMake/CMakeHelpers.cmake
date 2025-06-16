
macro(add_subdirectory_all_directories)

    file(GLOB MODULES_GLOB LIST_DIRECTORIES true "*")

    foreach(MODULE ${MODULES_GLOB})
        if(IS_DIRECTORY ${MODULE})
            add_subdirectory(${MODULE})
        endif()
    endforeach()

endmacro()

function(target_all_sources TARGET)

    file(GLOB_RECURSE GLOB_SOURCES FOLLOW_SYMLINKS
        "*.c"
        "*.cpp"
        "*.h"
        "*.hpp"
    )

    target_sources(${TARGET} PRIVATE ${GLOB_SOURCES})

endfunction()