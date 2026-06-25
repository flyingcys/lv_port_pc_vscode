# Task 1 红灯证据

目的：补充可独立核验的“先失败再实现”证据，不修改 `Task 1` 逻辑实现。

## 独立复现命令

下面命令会创建一个临时目录，只复制 `tests/clock.test.js`，故意不放 `script.js`，然后运行测试：

```bash
tmpdir="$(mktemp -d)"
mkdir -p "$tmpdir/tests"
cp tests/clock.test.js "$tmpdir/tests/clock.test.js"
cd "$tmpdir"
node --test tests/clock.test.js
```

预期结果：由于 `../script.js` 不存在，测试会以 `ERR_MODULE_NOT_FOUND` 失败。

## 本次实际复现输出

复现时间：`2026-06-19`

```text
Error [ERR_MODULE_NOT_FOUND]: Cannot find module '/private/var/folders/gj/c8y82nf53p5dh9cf1900g2700000gn/T/tmp.qraAvvh67a/script.js' imported from /private/var/folders/gj/c8y82nf53p5dh9cf1900g2700000gn/T/tmp.qraAvvh67a/tests/clock.test.js

Node.js v23.11.0
✖ tests/clock.test.js
ℹ tests 1
ℹ pass 0
ℹ fail 1
```

说明：`tmp.qraAvvh67a` 是本次临时目录名，每次复现都会变化；只要临时目录中不存在 `script.js`，失败性质应保持一致。
