# Task 1 报告

## 当前状态

Task 1 已完成，已验证通过，已提交。

## 修复说明

- 将 `AM_FILE_DIALOG_HELPER` 的执行方式从 `popen()`/shell 字符串改为 `fork + execvp`。
- 桌面对话框候选命令也改为 argv 直接执行，避免包含空格的参数被 shell 拆词。
- 保留并通过了带空格 helper 路径的测试用例。

## 本次验证

执行命令：

```bash
cmake --build build --target apple_music_desktop_file_dialog_test && \
ctest --test-dir build -R '^apple_music_desktop_file_dialog_test$' --output-on-failure
```

结果：通过

关键输出：

```text
Built target apple_music_desktop_file_dialog_test
1/1 Test #10: apple_music_desktop_file_dialog_test ...   Passed    0.02 sec
100% tests passed, 0 tests failed out of 1
```

## 备注

- 当前工作区仍有与 Task 1 无关的既有改动，未做回退。
- 本任务已只在 Task 1 写面内收敛修复。
