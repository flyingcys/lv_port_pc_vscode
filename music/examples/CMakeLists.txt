# 示例程序CMakeLists.txt

# 简单播放示例
add_executable(simple_player simple_player.c)
target_link_libraries(simple_player libaudio_player ${PLATFORM_LIBS})

# 事件回调示例
add_executable(event_example event_example.c)
target_link_libraries(event_example libaudio_player ${PLATFORM_LIBS})

# 网络流播放示例
add_executable(stream_player stream_player.c)
target_link_libraries(stream_player libaudio_player ${PLATFORM_LIBS})

# 环形缓冲区测试
add_executable(ringbuffer_test ringbuffer_test.c)
target_link_libraries(ringbuffer_test libaudio_player ${PLATFORM_LIBS})

# 安装示例
install(TARGETS simple_player event_example stream_player ringbuffer_test
        DESTINATION bin/examples) 