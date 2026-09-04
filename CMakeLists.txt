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
option(LV_USE_LIBPNG "Use libpng to decode PNG" OFF)
option(LV_USE_LIBJPEG_TURBO "Use libjpeg turbo to decode JPEG" OFF)
option(LV_USE_FFMPEG "Use libffmpeg to display video using lv_ffmpeg" OFF)
option(LV_USE_FREETYPE "Use freetype library" OFF)
set(VENUS_CODEGEN_DIR "" CACHE PATH "Generated Venus LVGL Codegen directory")
set(VENUS_LVGL_DIR "" CACHE PATH "LVGL source directory for Venus Codegen debug")
set(VENUS_CODEGEN_OUTPUT_DIR "" CACHE PATH "Output directory for Venus Codegen debug")

# Set C and C++ standards
set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set the output path for the executable
if(VENUS_CODEGEN_DIR)
    set(VENUS_CODEGEN_CMAKE "${VENUS_CODEGEN_DIR}/CMakeLists.txt")
    if(NOT EXISTS "${VENUS_CODEGEN_CMAKE}")
        message(FATAL_ERROR "VENUS_CODEGEN_DIR must contain CMakeLists.txt")
    endif()
    if(NOT VENUS_CODEGEN_OUTPUT_DIR)
        message(FATAL_ERROR "VENUS_CODEGEN_OUTPUT_DIR is required for Venus Codegen debug")
    endif()
    set(EXECUTABLE_OUTPUT_PATH "${VENUS_CODEGEN_OUTPUT_DIR}")
else()
    set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
endif()
set(WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})

set(PLATFORM_LINK_LIBS m pthread)
set(PLATFORM_INCLUDE_DIRS "")

# Emscripten uses its own SDL2 port instead of a host SDL2 package.
if(EMSCRIPTEN)
    set(EMSCRIPTEN_SDL_FLAGS "-s USE_SDL=2")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EMSCRIPTEN_SDL_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EMSCRIPTEN_SDL_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${EMSCRIPTEN_SDL_FLAGS}")
    set(PLATFORM_LINK_LIBS ${PLATFORM_LINK_LIBS})
else()
    # Find and include SDL2 library
    find_package(SDL2 REQUIRED)
    set(PLATFORM_INCLUDE_DIRS ${SDL2_INCLUDE_DIRS})
    list(APPEND PLATFORM_LINK_LIBS ${SDL2_LIBRARIES})
endif()

# Remove ARM-specific compile and linker options
if(NOT EMSCRIPTEN)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=native")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")
endif()

# Add compile definitions based on the selected options
add_compile_definitions($<$<BOOL:${LV_USE_DRAW_SDL}>:LV_USE_DRAW_SDL=1>)
add_compile_definitions($<$<BOOL:${LV_USE_LIBPNG}>:LV_USE_LIBPNG=1>)
add_compile_definitions($<$<BOOL:${LV_USE_LIBJPEG_TURBO}>:LV_USE_LIBJPEG_TURBO=1>)
add_compile_definitions($<$<BOOL:${LV_USE_FFMPEG}>:LV_USE_FFMPEG=1>)

# Add LVGL subdirectory
if(VENUS_LVGL_DIR)
    add_subdirectory("${VENUS_LVGL_DIR}" "${CMAKE_BINARY_DIR}/venus-lvgl")
else()
    add_subdirectory(lvgl)
endif()
target_include_directories(lvgl PUBLIC ${PROJECT_SOURCE_DIR} ${PLATFORM_INCLUDE_DIRS})

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
    if(VENUS_CODEGEN_DIR)
        add_executable(main ${PROJECT_SOURCE_DIR}/main/src/main.c)
    else()
        add_executable(main
            ${PROJECT_SOURCE_DIR}/main/src/main.c
            ${PROJECT_SOURCE_DIR}/main/src/mouse_cursor_icon.c
        )
    endif()
endif()

if(EMSCRIPTEN)
    set_target_properties(main PROPERTIES
        SUFFIX ".html"
        LINK_FLAGS "--shell-file ${PROJECT_SOURCE_DIR}/main/web_shell.html -s INITIAL_MEMORY=83886080 -s SINGLE_FILE=1"
    )
endif()

# Define LVGL configuration as a simple include
target_compile_definitions(main PRIVATE LV_CONF_INCLUDE_SIMPLE)
if(VENUS_CODEGEN_DIR)
    target_link_libraries(main lvgl ${PLATFORM_LINK_LIBS})
    include("${VENUS_CODEGEN_CMAKE}")
else()
    target_link_libraries(main lvgl lvgl::examples lvgl::demos lvgl::thorvg ${PLATFORM_LINK_LIBS})
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
