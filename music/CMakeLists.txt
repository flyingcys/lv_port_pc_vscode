cmake_minimum_required(VERSION 3.16)
project(AudioPlayer VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# 编译选项
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-g -O0 -Wall -Wextra)
else()
    add_compile_options(-O2 -Wall)
endif()

# 平台检测
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    add_definitions(-DPLATFORM_LINUX)
    set(PLATFORM_SOURCES
        src/platform/linux/thread.c
        src/platform/linux/file.c
        src/platform/linux/net.c
    )
    set(PLATFORM_LIBS pthread curl asound)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Generic")
    add_definitions(-DPLATFORM_ESP32)
    set(PLATFORM_SOURCES
        src/platform/esp32/thread.c
        src/platform/esp32/file.c
        src/platform/esp32/net.c
    )
    set(PLATFORM_LIBS)
endif()

# 头文件目录
include_directories(
    include
    src/core
    src/decoders
    src/sources
    src/outputs
    src/platform
    src/ui
    libs/minimp3
    libs/dr_libs
)

# 核心源文件
set(CORE_SOURCES
    src/core/ringbuffer.c
    src/core/player.c
    src/core/decoder.c
    src/core/source.c
    src/core/output.c
)

# 解码器源文件
set(DECODER_SOURCES
    src/decoders/mp3_decoder.c
    src/decoders/aac_decoder.c
    src/decoders/opus_decoder.c
    src/decoders/wav_decoder.c
)

# 音频源文件
set(SOURCE_SOURCES
    src/sources/file_source.c
    src/sources/http_source.c
    src/sources/m3u8_source.c
)

# 音频输出文件
set(OUTPUT_SOURCES
    src/outputs/alsa_output.c
    src/outputs/pulse_output.c
    src/outputs/i2s_output.c
)

# UI源文件
set(UI_SOURCES
    src/ui/cli.c
)

# 所有源文件
set(ALL_SOURCES
    ${CORE_SOURCES}
    ${DECODER_SOURCES}
    ${SOURCE_SOURCES}
    ${OUTPUT_SOURCES}
    ${UI_SOURCES}
    ${PLATFORM_SOURCES}
    src/main.c
)

# 创建可执行文件
add_executable(audio_player ${ALL_SOURCES})

# 链接库
target_link_libraries(audio_player ${PLATFORM_LIBS})

# 创建静态库
add_library(libaudio_player STATIC ${ALL_SOURCES})
set_target_properties(libaudio_player PROPERTIES OUTPUT_NAME audio_player)

# 安装
install(TARGETS audio_player DESTINATION bin)
install(FILES include/audio_player.h DESTINATION include)
install(TARGETS libaudio_player DESTINATION lib)

# 测试
enable_testing()
add_subdirectory(examples) 