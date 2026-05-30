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

file(GLOB MUSIC_PLAYER_SOURCES CONFIGURE_DEPENDS
    "${PROJECT_SOURCE_DIR}/main/src/v9_music_player/*.c"
)

# Set C and C++ standards
set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set the output path for the executable
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})

# Find and include SDL2 library
find_package(SDL2 REQUIRED)

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
        ${PROJECT_SOURCE_DIR}/main/src/mouse_cursor_icon.c
    )
endif()

target_sources(main PRIVATE ${MUSIC_PLAYER_SOURCES})

# Define LVGL configuration as a simple include
target_compile_definitions(main PRIVATE LV_CONF_INCLUDE_SIMPLE)
target_link_libraries(main lvgl lvgl::examples lvgl::demos lvgl::thorvg ${SDL2_LIBRARIES} m pthread)

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

add_executable(music_player_theme_test
    ${PROJECT_SOURCE_DIR}/main/tests/music_player_theme_test.c
    ${PROJECT_SOURCE_DIR}/main/src/v9_music_player/music_player_theme.c
)
target_include_directories(music_player_theme_test PRIVATE ${PROJECT_SOURCE_DIR})

add_executable(music_player_mock_data_test
    ${PROJECT_SOURCE_DIR}/main/tests/music_player_mock_data_test.c
    ${PROJECT_SOURCE_DIR}/main/src/v9_music_player/music_player_mock_data.c
)
target_include_directories(music_player_mock_data_test PRIVATE ${PROJECT_SOURCE_DIR})

add_executable(music_player_html_mockup_test
    ${PROJECT_SOURCE_DIR}/main/tests/music_player_html_mockup_test.c
)
target_include_directories(music_player_html_mockup_test PRIVATE ${PROJECT_SOURCE_DIR})

add_executable(music_player_shell_contract_test
    ${PROJECT_SOURCE_DIR}/main/tests/music_player_shell_contract_test.c
    ${MUSIC_PLAYER_SOURCES}
)
target_include_directories(music_player_shell_contract_test PRIVATE ${PROJECT_SOURCE_DIR})
target_compile_definitions(music_player_shell_contract_test PRIVATE LV_CONF_INCLUDE_SIMPLE)
set(MUSIC_PLAYER_TEST_LIBS
    lvgl
    lvgl::thorvg
    ${SDL2_LIBRARIES}
    ${PNG_LIBRARIES}
    m
    pthread
)
target_link_libraries(music_player_shell_contract_test ${MUSIC_PLAYER_TEST_LIBS})

add_executable(music_player_shell_lvgl_test
    ${PROJECT_SOURCE_DIR}/main/tests/music_player_shell_lvgl_test.c
    ${MUSIC_PLAYER_SOURCES}
)
target_include_directories(music_player_shell_lvgl_test PRIVATE ${PROJECT_SOURCE_DIR})
target_compile_definitions(music_player_shell_lvgl_test PRIVATE LV_CONF_INCLUDE_SIMPLE)
target_link_libraries(music_player_shell_lvgl_test ${MUSIC_PLAYER_TEST_LIBS})

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

add_test(
    NAME music_player_theme_test
    COMMAND $<TARGET_FILE:music_player_theme_test>
)
set_tests_properties(music_player_theme_test PROPERTIES WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})

add_test(
    NAME music_player_mock_data_test
    COMMAND $<TARGET_FILE:music_player_mock_data_test>
)
set_tests_properties(music_player_mock_data_test PROPERTIES WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})

add_test(
    NAME music_player_html_mockup_test
    COMMAND $<TARGET_FILE:music_player_html_mockup_test>
)
set_tests_properties(music_player_html_mockup_test PROPERTIES WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})

add_test(
    NAME music_player_shell_contract_test
    COMMAND $<TARGET_FILE:music_player_shell_contract_test>
)
set_tests_properties(music_player_shell_contract_test PROPERTIES WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})

add_test(
    NAME music_player_shell_lvgl_test
    COMMAND $<TARGET_FILE:music_player_shell_lvgl_test>
)
set_tests_properties(music_player_shell_lvgl_test PROPERTIES WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})

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
