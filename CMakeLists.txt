cmake_minimum_required(VERSION 3.20)
project(as2_map_stitcher VERSION 1.0.0 LANGUAGES CXX)

get_filename_component(COMPILER_BIN_DIR "${CMAKE_CXX_COMPILER}" DIRECTORY)
get_filename_component(COMPILER_ROOT_DIR "${COMPILER_BIN_DIR}" DIRECTORY)

set(MINGW_ZLIB_INCLUDE_DIR "${COMPILER_ROOT_DIR}/include")
set(MINGW_ZLIB_LIBRARY "${COMPILER_ROOT_DIR}/lib/libz.a")
if (EXISTS "${MINGW_ZLIB_INCLUDE_DIR}/zlib.h" AND
    EXISTS "${MINGW_ZLIB_LIBRARY}")
    add_library(as2_mingw_zlib STATIC IMPORTED GLOBAL)
    set_target_properties(as2_mingw_zlib PROPERTIES
        IMPORTED_LOCATION "${MINGW_ZLIB_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${MINGW_ZLIB_INCLUDE_DIR}"
    )
    set(AS2_ZLIB_TARGET as2_mingw_zlib)
else()
    find_package(ZLIB REQUIRED)
    set(AS2_ZLIB_TARGET ZLIB::ZLIB)
endif()

add_executable(as2_map_stitcher
    src/as2_map_stitcher/main.cpp
    src/as2_map_stitcher/bmp_reader.cpp
    src/as2_map_stitcher/png_writer.cpp
    src/as2_map_stitcher/map_stitcher.cpp
)

target_include_directories(as2_map_stitcher PRIVATE
    src/as2_map_stitcher
)

target_link_libraries(as2_map_stitcher PRIVATE ${AS2_ZLIB_TARGET})
target_compile_features(as2_map_stitcher PRIVATE cxx_std_17)
if (MSVC)
    target_compile_options(as2_map_stitcher PRIVATE /W4 /O2 /GL)
    target_link_options(as2_map_stitcher PRIVATE /LTCG /OPT:REF /OPT:ICF)
elseif (MINGW)
    target_compile_options(as2_map_stitcher PRIVATE -O2 -ffunction-sections -fdata-sections)
    target_link_options(as2_map_stitcher PRIVATE
        -static-libgcc
        -static-libstdc++
        -Wl,--gc-sections,--strip-all
    )

    # This CLion MinGW toolchain cannot reliably statically link libwinpthread.a.
    # Keep the DLL beside the executable so the result remains runnable.
    add_custom_command(TARGET as2_map_stitcher POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${COMPILER_BIN_DIR}/libwinpthread-1.dll"
            "$<TARGET_FILE_DIR:as2_map_stitcher>/libwinpthread-1.dll"
        COMMENT "Copying MinGW runtime DLL"
    )
endif()
set_target_properties(as2_map_stitcher PROPERTIES OUTPUT_NAME AS2MapStitcher)
