# CMake generated Testfile for 
# Source directory: /Users/cys/embedded/lv_port_pc_vscode
# Build directory: /Users/cys/embedded/lv_port_pc_vscode/build-mac-debug
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(fruit_ninja_assets_test "/Users/cys/embedded/lv_port_pc_vscode/bin/fruit_ninja_assets_test")
set_tests_properties(fruit_ninja_assets_test PROPERTIES  WORKING_DIRECTORY "/Users/cys/embedded/lv_port_pc_vscode" _BACKTRACE_TRIPLES "/Users/cys/embedded/lv_port_pc_vscode/CMakeLists.txt;246;add_test;/Users/cys/embedded/lv_port_pc_vscode/CMakeLists.txt;0;")
add_test(fruit_ninja_collision_test "/Users/cys/embedded/lv_port_pc_vscode/bin/fruit_ninja_collision_test")
set_tests_properties(fruit_ninja_collision_test PROPERTIES  WORKING_DIRECTORY "/Users/cys/embedded/lv_port_pc_vscode" _BACKTRACE_TRIPLES "/Users/cys/embedded/lv_port_pc_vscode/CMakeLists.txt;252;add_test;/Users/cys/embedded/lv_port_pc_vscode/CMakeLists.txt;0;")
add_test(fruit_ninja_model_test "/Users/cys/embedded/lv_port_pc_vscode/bin/fruit_ninja_model_test")
set_tests_properties(fruit_ninja_model_test PROPERTIES  WORKING_DIRECTORY "/Users/cys/embedded/lv_port_pc_vscode" _BACKTRACE_TRIPLES "/Users/cys/embedded/lv_port_pc_vscode/CMakeLists.txt;258;add_test;/Users/cys/embedded/lv_port_pc_vscode/CMakeLists.txt;0;")
subdirs("third-party/hls_player_demo/src/http_client")
subdirs("third-party/hls_player_demo/src/player_controller")
subdirs("third-party/hls_player_demo/src/stream_player")
subdirs("lvgl")
