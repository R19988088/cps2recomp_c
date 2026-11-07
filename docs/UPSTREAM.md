# 跟随上游更新（fork 维护说明）

本仓库是 [R19988088/cps2recomp_c](https://github.com/R19988088/cps2recomp_c) 的 fork，在它的基础上加了
**两次改动**：ROM 扫描（去掉自动下载，改成选文件夹 + 刷新）和**中文界面**。

这份说明只讲一件事：**以后上游更新时，怎么在不丢掉本仓库改动的前提下把上游的功能拿过来。**

## 一、两个仓库的关系

上游 `R19988088/cps2recomp_c` 本身是 [tmcleroy/cps2recomp](https://github.com/tmcleroy/cps2recomp) 的镜像，
两者没有分叉：`R19988088` 的 main **是** `tmcleroy` 的祖先，所以直接 fast-forward 就能跟上。跟哪一条都可以，
建议跟 `R19988088`，因为它只往上放完整的、已验证的更新。

本仓库的提交都建立在 3f83e1d（上游 1.3.0）之上。

## 二、两个目录，两种跟随方式

这个项目里有两类代码，跟随方式**完全不同**，这是最容易出错的地方：

| 目录 | 是什么 | 能不能直接改 |
| --- | --- | --- |
| 仓库其余部分（`port/`、`launcher/`、`games/`、`tools/`…） | 本 fork 的代码 | 可以，正常提交 |
| `recomp-common/` | 另一个项目 [recomp-common](https://github.com/tmcleroy/recomp-common) 的**只读副本** | **不能**，`make check` 会用哈希校验并失败 |

### `recomp-common/` 的正确改法

它由 `tools/sync-common.sh` 从旁边的 `../recomp-common` 克隆复制而来（默认在
仓库同级的 `../recomp-common`，分支 `fork`）。要改启动器界面、菜单这类共享代码：

```sh
cd ../recomp-common
# 在 fork 分支上改，然后提交
git add -A && git commit -m "..."
cd <本仓库>
RECOMP_COMMON=../recomp-common tools/sync-common.sh   # 同步回来
tools/sync-common.sh --check   # 确认一致性（CI 也跑这个）
```

**注意**：`sync-common.sh` 只复制**已提交**的内容。改完不提交就同步，改动不会进本仓库。

## 三、日常跟随上游

```sh
cd <本仓库>
git fetch upstream
git merge --ff-only upstream/main      # 能快进就快进
```

能快进说明你本地没有独立提交，直接跟上了。**如果提示无法 fast-forward**（因为你本地有提交，必然如此），
改用 rebase，把本地改动挪到上游最新之上：

```sh
git rebase upstream/main
```

冲突只会出现在少数「接缝」文件上，因为本 fork 的设计刻意减少了接触面：

- `launcher/CMakeLists.txt`（改过更新源和源文件列表）
- `port/main.cpp`（加了 i18n 的引入）
- `CMakeLists.txt`（加了 `i18n.cpp` 和 `i18n_table.cpp`）
- `launcher/src/description.cpp`（加了翻译表挂载）

**中文文案表 `port/i18n_table.cpp` 不会冲突**——它是本仓库独有的新文件，上游没有。

## 四、上游改了 `recomp-common` 怎么办

上游偶尔会把 `recomp-common` pin 到更新的提交。这时要先把自己的 fork 也更新，再同步：

```sh
cd ../recomp-common
git fetch origin
git rebase origin/main        # 把 fork 分支（含 i18n 改动）挪到上游最新
cd <本仓库>
RECOMP_COMMON=../recomp-common tools/sync-common.sh
```

如果 rebase 冲突，冲突集中在这些文件：`src/settings.cpp`（翻译挂载）、`src/i18n.*`（新文件，不会冲突）、
`launcher/src/app.cpp`（语言设置行、ROM 行布局）。

## 五、自动更新与本地修改的关系

启动器的自动更新功能会用**发布版的归档覆盖启动器自己的文件**。本仓库已经把它默认关闭
（`launcher/CMakeLists.txt` 里 `LAUNCHER_UPDATE_REPO` 为空）：

```cmake
set(LAUNCHER_UPDATE_REPO "" CACHE STRING "owner/name whose releases this build updates itself from (empty: none)")
```

**这意味着**：本仓库构建出的 app 不会自动更新。这是刻意的——否则它会把你自己的中文版覆盖成上游英文版。

如果要给自己的发布版开启自动更新，必须指向**你自己的**发布仓库：

```sh
cmake -DLAUNCHER_UPDATE_REPO=你的账号/你的仓库 ...
```

并且那个仓库的发布流程要生成 `update-macos.txt` 等清单（见 `tools/release/`），否则更新会失败。

## 六、验证

改完一定要跑：

```sh
make check          # audit + recomp-common 校验 + 冒烟测试
make GAME=<id> check   # 某个游戏的翻译代码与解释器逐帧比对
```

`make check` 里的 audit 会拦截：游戏数据、密钥、个人路径、超过 512KB 的文件。
注意它也会拦截指向 ROM 下载站的地址——不要提交这类链接。
