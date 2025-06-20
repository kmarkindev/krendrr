
FetchContent_Declare(
    stb
    GIT_REPOSITORY https://github.com/nothings/stb.git
    GIT_TAG f58f558c120e9b32c217290b80bad1a0729fbb2c # master, May 26 2025
)
FetchContent_MakeAvailable(stb)

macro(setup_stb_module STB_NAME)

    add_library(krendrr_thirdparty_${STB_NAME} STATIC)
    add_library(krendrr::thirdparty::${STB_NAME} ALIAS krendrr_thirdparty_${STB_NAME})

    file(COPY
            ${stb_SOURCE_DIR}/${STB_NAME}.h
            DESTINATION
            ${CMAKE_CURRENT_BINARY_DIR}/include/${STB_NAME}
    )
    target_include_directories(krendrr_thirdparty_${STB_NAME} PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/include/${STB_NAME})
    target_sources(krendrr_thirdparty_${STB_NAME} PRIVATE ${STB_NAME}.cpp)

endmacro()

setup_stb_module(stb_image)
