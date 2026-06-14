cmake_minimum_required(VERSION 3.10)
project(lvgl C CXX)

# Set the correct FreeRTOS port for your system (e.g., Posix for WSL)
set(FREERTOS_PORT GCC_POSIX CACHE STRING "Port for FreeRTOS on Posix environment")

unset(USE_FREERTOS CACHE)

option(USE_FREERTOS "Enable FreeRTOS" OFF) # Turn this on to enable FreeRTOS

if(USE_FREERTOS)
    message(STATUS "FreeRTOS is enabled")

    # FreeRTOS integration when USE_FREERTOS is enabled
    add_library(freertos_config INTERFACE)
    target_include_directories(freertos_config SYSTEM INTERFACE ${PROJECT_SOURCE_DIR}/config)
    target_compile_definitions(freertos_config INTERFACE projCOVERAGE_TEST=0)

    # Add FreeRTOS as a subdirectory
    add_subdirectory(FreeRTOS)

    # FreeRTOS-specific include directories
    include_directories(${PROJECT_SOURCE_DIR}/FreeRTOS/include)
    include_directories(${PROJECT_SOURCE_DIR}/FreeRTOS/portable/ThirdParty/GCC/Posix)
    include_directories(${PROJECT_SOURCE_DIR}/config)

    # Add FreeRTOS sources
    file(GLOB FREERTOS_SOURCES
        "${PROJECT_SOURCE_DIR}/FreeRTOS/*.c"
        "${PROJECT_SOURCE_DIR}/FreeRTOS/portable/MemMang/heap_4.c"
        "${PROJECT_SOURCE_DIR}/FreeRTOS/portable/ThirdParty/GCC/Posix/*.c"
    )
else()
    message(STATUS "FreeRTOS is disabled")
    set(FREERTOS_SOURCES "")  # No FreeRTOS sources if FreeRTOS is disabled
endif()

# Main include files of the project
include_directories(${PROJECT_SOURCE_DIR}/main/inc)

# Define options for LVGL with default values (OFF)
option(LV_USE_DRAW_SDL "Use SDL draw unit" OFF)
option(LV_USE_LIBPNG "Use libpng to decode PNG" ON)
option(LV_USE_LIBJPEG_TURBO "Use libjpeg turbo to decode JPEG" OFF)
option(LV_USE_FFMPEG "Use libffmpeg to display video using lv_ffmpeg" OFF)
option(LV_USE_FREETYPE "Use freetype library" OFF)

set(LV_USE_LIBPNG ON CACHE BOOL "Use libpng to decode PNG" FORCE)
set(LV_USE_LIBJPEG_TURBO OFF CACHE BOOL "Use libjpeg turbo to decode JPEG" FORCE)

file(GLOB FRUIT_NINJA_SOURCES CONFIGURE_DEPENDS
    "${PROJECT_SOURCE_DIR}/main/src/v9-fruit_ninja/*.c"
)

file(GLOB APPLE_MUSIC_SOURCES CONFIGURE_DEPENDS
    "${PROJECT_SOURCE_DIR}/main/src/v9_apple_music/*.c")
file(GLOB APPLE_MUSIC_FONTS CONFIGURE_DEPENDS
    "${PROJECT_SOURCE_DIR}/main/src/v9_apple_music/fonts/*.c")

# Set C and C++ standards
set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set the output path for the executable
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})

find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)
find_package(ALSA REQUIRED)

set(HLS_PROJECT_ROOT_DIR ${PROJECT_SOURCE_DIR}/third-party/hls_player_demo)

add_subdirectory(${HLS_PROJECT_ROOT_DIR}/src/http_client)
add_subdirectory(${HLS_PROJECT_ROOT_DIR}/src/player_controller)
add_subdirectory(${HLS_PROJECT_ROOT_DIR}/src/stream_player)

# Find and include SDL2 library
find_package(SDL2 REQUIRED)
find_package(SDL2_mixer QUIET)

if(NOT SDL2_mixer_FOUND AND NOT TARGET SDL2_mixer::SDL2_mixer AND NOT TARGET SDL2_mixer::SDL2_mixer-static)
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(SDL2_MIXER QUIET SDL2_mixer)
    endif()

    find_path(SDL2_MIXER_INCLUDE_DIR
        NAMES SDL_mixer.h
        PATH_SUFFIXES SDL2
        HINTS ${SDL2_MIXER_INCLUDE_DIRS}
    )
    find_library(SDL2_MIXER_LIBRARY
        NAMES SDL2_mixer
        HINTS ${SDL2_MIXER_LIBRARY_DIRS}
    )

    if(SDL2_MIXER_INCLUDE_DIR AND SDL2_MIXER_LIBRARY)
        set(SDL2_MIXER_INCLUDE_DIRS ${SDL2_MIXER_INCLUDE_DIR})
        set(SDL2_MIXER_LIBRARIES ${SDL2_MIXER_LIBRARY})
    endif()
endif()

if(NOT SDL2_mixer_FOUND AND NOT TARGET SDL2_mixer::SDL2_mixer AND NOT TARGET SDL2_mixer::SDL2_mixer-static AND NOT SDL2_MIXER_LIBRARIES)
    message(FATAL_ERROR "SDL2_mixer is required to play Fruit Ninja audio. Install SDL2_mixer and retry.")
endif()

# Remove ARM-specific compile and linker options
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=native")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")

# Add compile definitions based on the selected options
add_compile_definitions($<$<BOOL:${LV_USE_DRAW_SDL}>:LV_USE_DRAW_SDL=1>)
add_compile_definitions($<$<BOOL:${LV_USE_LIBPNG}>:LV_USE_LIBPNG=1>)
add_compile_definitions($<$<BOOL:${LV_USE_LIBJPEG_TURBO}>:LV_USE_LIBJPEG_TURBO=1>)
add_compile_definitions($<$<BOOL:${LV_USE_FFMPEG}>:LV_USE_FFMPEG=1>)

# Add LVGL subdirectory
add_subdirectory(lvgl)
target_include_directories(lvgl PUBLIC ${PROJECT_SOURCE_DIR} ${SDL2_INCLUDE_DIRS})

# Create the main executable, depending on the FreeRTOS option
if(USE_FREERTOS)
    add_executable(main
        ${PROJECT_SOURCE_DIR}/main/src/main.c
        ${PROJECT_SOURCE_DIR}/main/src/freertos_main.cpp
        ${PROJECT_SOURCE_DIR}/main/src/mouse_cursor_icon.c
        ${PROJECT_SOURCE_DIR}/main/src/FreeRTOS_Posix_Port.c
        ${FREERTOS_SOURCES}  # Add only if USE_FREERTOS is enabled
    )
    # Link FreeRTOS libraries
    target_link_libraries(main freertos_config FreeRTOS)
else()
    add_executable(main
        ${PROJECT_SOURCE_DIR}/main/src/main.c
        ${PROJECT_SOURCE_DIR}/main/src/music_player.c
        ${PROJECT_SOURCE_DIR}/main/src/mouse_cursor_icon.c
    )

    target_include_directories(main PRIVATE
        ${HLS_PROJECT_ROOT_DIR}/src
        ${HLS_PROJECT_ROOT_DIR}/src/stream_player/include
        ${HLS_PROJECT_ROOT_DIR}/src/player_controller
        ${HLS_PROJECT_ROOT_DIR}/src/http_client
        ${ALSA_INCLUDE_DIRS}
    )

    target_compile_definitions(main PRIVATE
        PLATFORM_LINUX
        OPUS_BUILD
        VAR_ARRAYS
    )
endif()

target_sources(main PRIVATE ${FRUIT_NINJA_SOURCES})
target_sources(main PRIVATE ${APPLE_MUSIC_SOURCES} ${APPLE_MUSIC_FONTS})

# Define LVGL configuration as a simple include
target_compile_definitions(main PRIVATE LV_CONF_INCLUDE_SIMPLE)
target_link_libraries(main lvgl lvgl::examples lvgl::demos lvgl::thorvg ${SDL2_LIBRARIES} m pthread
    player_controller
    stream_player_module_linux
    ${ALSA_LIBRARIES}
)

if(TARGET SDL2_mixer::SDL2_mixer)
    target_link_libraries(main SDL2_mixer::SDL2_mixer)
elseif(TARGET SDL2_mixer::SDL2_mixer-static)
    target_link_libraries(main SDL2_mixer::SDL2_mixer-static)
else()
    target_include_directories(main PRIVATE ${SDL2_MIXER_INCLUDE_DIRS})
    target_link_libraries(main ${SDL2_MIXER_LIBRARIES})
endif()

# Only link freertos_config if the FreeRTOS directory exists
if(USE_FREERTOS)
    target_link_libraries(main freertos_config)
endif()

# Custom target to run the executable
add_custom_target(run COMMAND ${EXECUTABLE_OUTPUT_PATH}/main DEPENDS main)

# Conditionally include and link SDL2_image if LV_USE_DRAW_SDL is enabled
if(LV_USE_DRAW_SDL)
    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")
    find_package(SDL2_image REQUIRED)
    target_include_directories(lvgl PUBLIC ${SDL2_IMAGE_INCLUDE_DIRS})
    target_link_libraries(main ${SDL2_IMAGE_LIBRARIES})
endif()

# Conditionally include and link libpng if LV_USE_LIBPNG is enabled
if(LV_USE_LIBPNG)
    find_package(PNG REQUIRED)
    target_include_directories(lvgl PUBLIC ${PNG_INCLUDE_DIRS})
    target_link_libraries(main ${PNG_LIBRARIES})
endif()

# Conditionally include and link libjpeg-turbo if LV_USE_LIBJPEG_TURBO is enabled
if(LV_USE_LIBJPEG_TURBO)
    find_package(JPEG REQUIRED)
    target_include_directories(lvgl PUBLIC ${JPEG_INCLUDE_DIRS})
    target_link_libraries(main ${JPEG_LIBRARIES})
endif()

# Conditionally include and link FFmpeg libraries if LV_USE_FFMPEG is enabled
if(LV_USE_FFMPEG)
    target_link_libraries(main avformat avcodec avutil swscale z)
endif()

# Conditionally include and link FreeType if LV_USE_FREETYPE is enabled
if(LV_USE_FREETYPE)
    find_package(Freetype REQUIRED)
    target_include_directories(lvgl PUBLIC ${FREETYPE_INCLUDE_DIRS})
    target_link_libraries(main ${FREETYPE_LIBRARIES})
endif()

enable_testing()

add_executable(fruit_ninja_assets_test
    ${PROJECT_SOURCE_DIR}/main/tests/fruit_ninja_assets_test.c
    ${PROJECT_SOURCE_DIR}/main/src/v9-fruit_ninja/fruit_ninja_assets.c
)
target_include_directories(fruit_ninja_assets_test PRIVATE ${PROJECT_SOURCE_DIR})

add_executable(fruit_ninja_collision_test
    ${PROJECT_SOURCE_DIR}/main/tests/fruit_ninja_collision_test.c
    ${PROJECT_SOURCE_DIR}/main/src/v9-fruit_ninja/fruit_ninja_collision.c
)
target_include_directories(fruit_ninja_collision_test PRIVATE ${PROJECT_SOURCE_DIR})
target_link_libraries(fruit_ninja_collision_test m)

add_executable(fruit_ninja_model_test
    ${PROJECT_SOURCE_DIR}/main/tests/fruit_ninja_model_test.c
)
target_include_directories(fruit_ninja_model_test PRIVATE ${PROJECT_SOURCE_DIR})

add_test(
    NAME fruit_ninja_assets_test
    COMMAND $<TARGET_FILE:fruit_ninja_assets_test>
)
set_tests_properties(fruit_ninja_assets_test PROPERTIES WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})

add_test(
    NAME fruit_ninja_collision_test
    COMMAND $<TARGET_FILE:fruit_ninja_collision_test>
)
set_tests_properties(fruit_ninja_collision_test PROPERTIES WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})

add_test(
    NAME fruit_ninja_model_test
    COMMAND $<TARGET_FILE:fruit_ninja_model_test>
)
set_tests_properties(fruit_ninja_model_test PROPERTIES WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})

add_executable(apple_music_data_test
    ${PROJECT_SOURCE_DIR}/main/tests/apple_music_data_test.c
    ${PROJECT_SOURCE_DIR}/main/src/v9_apple_music/am_data.c
)
target_include_directories(apple_music_data_test PRIVATE ${PROJECT_SOURCE_DIR})
add_test(NAME apple_music_data_test COMMAND $<TARGET_FILE:apple_music_data_test>)
set_tests_properties(apple_music_data_test PROPERTIES WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})

add_executable(apple_music_theme_test
    ${PROJECT_SOURCE_DIR}/main/tests/apple_music_theme_test.c
    ${PROJECT_SOURCE_DIR}/main/src/v9_apple_music/am_theme.c
)
target_include_directories(apple_music_theme_test PRIVATE ${PROJECT_SOURCE_DIR})
target_compile_definitions(apple_music_theme_test PRIVATE LV_CONF_INCLUDE_SIMPLE)
target_link_libraries(apple_music_theme_test lvgl)
add_test(NAME apple_music_theme_test COMMAND $<TARGET_FILE:apple_music_theme_test>)
set_tests_properties(apple_music_theme_test PROPERTIES WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})

add_executable(apple_music_metrics_test
    ${PROJECT_SOURCE_DIR}/main/tests/apple_music_metrics_test.c
    ${PROJECT_SOURCE_DIR}/main/src/v9_apple_music/am_metrics.c
    ${PROJECT_SOURCE_DIR}/main/tests/am_fonts_stub.c)
target_include_directories(apple_music_metrics_test PRIVATE ${PROJECT_SOURCE_DIR})
target_compile_definitions(apple_music_metrics_test PRIVATE LV_CONF_INCLUDE_SIMPLE)
target_link_libraries(apple_music_metrics_test lvgl)
add_test(NAME apple_music_metrics_test COMMAND $<TARGET_FILE:apple_music_metrics_test>)
set_tests_properties(apple_music_metrics_test PROPERTIES WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})

add_custom_target(fruit_ninja_tests
    COMMAND $<TARGET_FILE:fruit_ninja_assets_test>
    COMMAND $<TARGET_FILE:fruit_ninja_collision_test>
    COMMAND $<TARGET_FILE:fruit_ninja_model_test>
    DEPENDS fruit_ninja_assets_test fruit_ninja_collision_test fruit_ninja_model_test
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
)

# Apply additional compile options if the build type is Debug
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Debug mode enabled")

    target_compile_options(lvgl PRIVATE
        -pedantic-errors
        -Wall
        -Wclobbered
        -Wdeprecated
        -Wdouble-promotion
        -Wempty-body
        -Wextra
        -Wformat-security
        -Wmaybe-uninitialized
        # -Wmissing-prototypes
        -Wpointer-arith
        -Wmultichar
        -Wno-pedantic # ignored for now, we convert functions to pointers for properties table.
        -Wreturn-type
        -Wshadow
        -Wshift-negative-value
        -Wsizeof-pointer-memaccess
        -Wtype-limits
        -Wundef
        -Wuninitialized
        -Wunreachable-code
        -Wfloat-conversion
        -Wstrict-aliasing
    )

if (ASAN)
    message(STATUS "AddressSanitizer enabled")

    # Add AddressSanitizer flags
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fsanitize=address")
else()
    message(STATUS "AddressSanitizer disabled")
endif()
endif()
