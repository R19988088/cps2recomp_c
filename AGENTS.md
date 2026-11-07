# AGENTS.md

本文件是本仓库对 AI 编码代理的说明。**每次修改前先读这里。**

## 用户要求（优先级最高）

1. **只关注用户需求。** 不主动扩大范围，不做用户没要求的重构、清理、"顺手优化"。改动以解决用户提出的问题为准。
2. **优先跨平台。** 任何改动都要在 macOS / Linux（含 Steam Deck、ARM64）/ Windows / Android 上成立。优先使用可移植的 API，不用平台专属写法；确实必须分平台时，用项目已有的条件编译方式（`#ifdef _WIN32`、`APPLE`、`ANDROID` 等）并保持各平台行为一致。
3. **优先中文。** 面向用户的文本（菜单、提示、错误信息、文档、界面）优先用中文；代码标识符、注释沿用仓库原有风格（英文），不要为了汉化去改标识符或内部注释。中文文本必须用 UTF-8，并在所有目标平台上正确显示（注意 Windows 控制台编码、SDL/ImGui 字体需包含 CJK 字形）。

## 项目是什么

CPS-2 Recomp：把 Capcom CPS-2（以及部分 CPS-1）街机游戏的主程序**提前翻译成 C++**，运行在本项目自己写的机器模型上——不是模拟器。

- 上游仓库：https://github.com/R19988088/cps2recomp_c （原始项目为 https://github.com/tmcleroy/cps2recomp）
- 版本：见 `VERSION`（当前 1.3.0）
- 许可证：GPL-3.0-only

## 目录结构

| 目录 | 作用 |
| --- | --- |
| `port/` | 平台前端：主循环、SDL 前端、GPU 画面、菜单、HUD、触屏、联机、音轨、macOS 入口 |
| `machine/` | 自研的 CPS-2 机器：68000、视频、QSound、声音板、EEPROM、ROM 集、CPS-2 解密、Kabuki |
| `runtime/` | 运行时：游戏循环、板级时序、翻译代码的调度、状态哈希 |
| `recomp-common/` | 与 satrecomp 共享的代码（菜单、设置、启动器）；由 `tools/sync-common.sh` 同步 |
| `fight/` | 格斗游戏功能：训练模式、帧数据、配色编辑、对战、组队/Marvel、客串场地 |
| `beat/` | 清版动作游戏的功能与模型查看器 |
| `shmup/` | 射击游戏的功能 |
| `model/` | 3D 模型查看器 |
| `video/` | 原生视频渲染、widescreen、shader、lift |
| `sound/` | Z80、QSound、声音驱动 |
| `games/<id>/` | 每个游戏的数据：`game.cfg`、`src/`（手写 C++ 重写）、录像、检查脚本、文档 |
| `launcher/` | 启动器描述信息（`src/description.cpp`） |
| `online/` | 联机大厅服务（Cloudflare Worker） |
| `tools/` | 工具链：翻译器 `recomp.py`、`recompz80.py`、`m68k.py`、检查脚本、发布脚本 |
| `docs/` | 项目文档（构建、时序、联机、训练模式、画面、工具链等） |
| `android/` | Android 工程 |
| `third_party/` | 第三方单头文件库（解码器、librashader 等），不修改 |
| `example/` | 无 ROM 时用于 CI 的替身游戏 |

生成物与被忽略的目录：`work/<game>/gen/`（翻译出的 C++）、`build/`、`rom/`、`*.zip`、`*.key`、`*.state`。

## 构建

依赖：CMake 3.28+、Ninja、SDL3 3.4.16+、Python 3；macOS 额外需要 Xcode 命令行工具。

```sh
make GAME=<id>            # 翻译并构建（全新目录第一次要跑两遍）
make GAME=<id> explore    # 首次约十分钟：通过模拟游玩找出游戏代码并翻译
make collection           # 构建 rom/ 下所有游戏 + 启动器
make check                # audit + recomp-common 校验 + smoke
make GAME=<id> check      # 翻译代码与解释器逐状态哈希对比
make GAME=<id> check-rewrites   # 逐次调用校验手写重写
make android              # 构建 Android APK
```

把 ROM 放在 `rom/<id>/`（或 `ROM=/path` 指定），**ROM 永不提交**。

本机环境（macOS）：CMake 4.4.0、Ninja 1.13.2、Python 3.9.6、SDL3 **3.4.20**。
`CMakeLists.txt:25` 要求 SDL3 ≥ 3.4.16（macOS 上 3.4.14 会让窗口全黑），本机已满足。构建与验证命令：

```sh
make smoke          # example 的构建 + 程序/启动器启动检查（CI 用同一套），最快的回归验证
make audit          # 仓库内容检查，需要先 git add
```

## 硬性约束

- **绝不提交任何游戏数据**：ROM、从 ROM 派生的内容、密钥、存档、录像里的游戏数据。仓库只含代码。
- 不改 `third_party/` 下的文件。
- 改 `recomp-common/` 外的东西时，注意它是一份副本；修改共享代码要走 `tools/sync-common.sh` 的流程。
- 手写的游戏逻辑重写必须与其替换的原例程行为完全一致（寄存器、标志、写过的内存），并接受 `--check-rewrites` 校验。

## 代码风格

- C++20，缩进 4 空格，行宽约 120。
- 注释写"这条代码本身表达不了的约束"，不要写"下一行做什么"或"为什么这个改动是对的"。
- 新增逻辑优先复用已有抽象（`runtime/lift.h`、`port/port.h`、`recomp-common`），不要另起一套。
- 用户可见文本集中管理，便于中文/多语言；新增文案默认给中文，必要时保留英文键名。

## 文档

- 用户向：`README.md`、`docs/*.md`、`games/<id>/README.md`
- 开发向：`docs/TOOLKIT.md`（工具链与新增游戏）、`docs/READABLE.md`（如何写可读的重写代码）、`docs/BUILDING.md`
- 改行为时同步更新对应文档与 `CHANGELOG.md`。
