# lv_nes 集成说明

## 主要调整
- Makefile 增加 `lv_nes/src` 头文件搜索路径，并排除 `lv_nes/src/main.c`，避免重复入口与编译缺失头文件。
- `main/src/main.c` 引入 `lv_nes`，初始化 STDIO 文件系统驱动（`lv_fs_stdio_init`），并将默认示例替换为 `lv_nes_simple_test()`，直接进入 NES 界面。
- `lv_nes/src/lv_nes.c` 的 ROM 路径改为 `A:lv_nes/src/dummy.nes`，与工程内置 ROM 对齐并使用 LVGL FS 接口读取。
- `lv_drv_conf.h` 调整 X11 分辨率为 640x480，匹配 256x240 的 NES 画面（2x 缩放）以避免裁切。

## 构建与运行
- 构建：在工程根目录执行 `make`，产物位于 `build/bin/demo`。
- 运行：`./build/bin/demo`（当前配置使用 X11 驱动）。
- 按键：界面底部按钮对应 NES 手柄，加载的默认 ROM 为 `lv_nes/src/dummy.nes`。

## 注意事项
- 依赖 `lv_conf.h` 中的 `LV_USE_FS_STDIO` 和文件系统初始化，若调整文件系统配置，请同步更新 ROM 路径或驱动初始化。
- 如需更换 ROM，可将新文件放置于工程目录并修改 `lv_nes/src/lv_nes.c` 中的路径。
