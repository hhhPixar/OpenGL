# macOS 适配与构建运行指南

> 本项目是一个 TheCherno/Hazel 风格的 OpenGL 引擎(premake5 + gmake,原为 Windows/VS2019),
> 现已移植到 Apple Silicon macOS。本文记录 **如何编译、如何运行、本次适配改了哪些内容**,
> 以及移植过程中踩到的 macOS 专属坑(GL 驱动楔死等)。
>
> 仓库根目录:`/Users/tal/CPP/OpenGL`
> 目标机器:Apple Silicon(M3),macOS 最高支持 OpenGL 4.1(经 Metal 转译)。

---

## 一、macOS 适配要点(为什么改这些)

macOS 的 OpenGL 实现**最高只有 4.1**(且 Apple 已弃用,经 GL→Metal 转译),而原项目按
Windows 的 4.5 写。两者差距主要落在两件事:① 高版本 GL 调用会把 Metal 驱动**楔死**;
② 工具链从 MSVC 换成 clang。下面逐条说明。

### 1. OpenGL 4.5 DSA 调用 → 改回 3. 的 glGen*(会楔死驱动)

`glCreateVertexArrays` / `glCreateBuffers` / `glCreateTextures` 等是 GL 4.5 的 DSA
(Direct State Access)接口,macOS 4.1 不支持。在 macOS 的 GL→Metal 转译层上调用它们,
**会把 Metal 驱动楔死**:进程进入 `U`(不可中断等待)状态,`kill -9` 杀不掉,只能**重启 Mac**。

- 修法:改回 GL 3.0 的 `glGenVertexArrays` / `glGenBuffers`,紧跟 `glBindVertexArray` /
  `glBindBuffer` 即可,效果等价。
- 涉及文件:`OpenGL-Examples/src/ExampleLayer.cpp`(`m_QuadVA`/`m_QuadVB`/`m_QuadIB` 三处)。

### 2. OpenGL 4.3 KHR_debug 调试输出 → macOS 上做空操作(同样会楔死)

`EnableGLDebugging()` 里的 `glDebugMessageCallback` / `glEnable(GL_DEBUG_OUTPUT)` /
`glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS)` 都是 GL 4.3(KHR_debug)特性,macOS 4.1 不支持,
**和 DSA 一样会楔死 Metal 驱动**。这是 DSA 之外的第二个 macOS 楔死点。

- 修法:在 `OpenGLCore/Util/OpenGLDebug.cpp::EnableGLDebugging` 里用 `#ifndef __APPLE__`
  把整个函数体包起来,macOS 上做空操作(和 ImGui 多视口同款处理)。
- 典型症状:**窗口出来了但全黑/无图像,进程处于 `U` 状态** —— 基本就是卡在 `OnAttach`
  阶段某个 ≥4.2 的 GL 调用上。用 stderr trace 定位。

### 3. GLSL `#version` 必须 410

- `test.vert.glsl` / `test.frag.glsl`:`#version 450 core` → `#version 410 core`。
- macOS 4.1 对应的 GLSL 就是 410,用 450 会编译失败。

### 4. GL 上下文 hint:3.3 Core + FORWARD_COMPAT

创建窗口前必须设置,否则 macOS 只给 legacy 2.1 上下文,ImGui 的 GL3 后端(`#version 410`)
需要 3.2+ core:

```cpp
glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);   // macOS 必须加
#endif
```

- 涉及文件:`OpenGL-Core/src/Platform/Windows/WindowsWindow.cpp`(`Init` 内,`glfwCreateWindow` 之前)。

### 5. ImGui 多视口:macOS 关掉

`ImGuiConfigFlags_ViewportsEnable` 在 macOS Metal 上不稳定(渲染异常/窗口无法关闭),
用 `#ifndef __APPLE__` 关闭。涉及 `ImGuiLayer.cpp`。

### 6. premake gmake 不传播静态库的 links 给消费者

`OpenGL-Core` links 了 GLFW/Glad/ImGui,但 gmake 工具链**不会**把这个静态库的依赖传递给
Sandbox/Examples。所以:

- `OpenGL-Core/premake5.lua`:把 `opengl32` 从公共 `links` 移到 Windows filter 内;
  新增 macOS filter(`defines { GLCORE_PLATFORM_MACOS, GLFW_INCLUDE_NONE }`)。
- `OpenGL-Examples/premake5.lua` 和 `OpenGL-Sandbox/premake5.lua`:新增 macOS filter,显式
  `links { "GLFW", "Glad", "ImGui" }`,并加系统框架 `linkoptions`:

  ```lua
  linkoptions { "-framework OpenGL", "-framework Cocoa",
                "-framework CoreVideo", "-framework IOKit" }
  ```

  (这些框架要由最终可执行文件链接,不能放进 Core 静态库。)

### 7. MSVC → clang 源码兼容

| 文件 | 改动 |
| --- | --- |
| `Core/Core.h` | `__debugbreak()` → 分平台宏 `GLCORE_DEBUGBREAK()`(MSVC 用 `__debugbreak()`,其它用 `__builtin_debugtrap()`);`GLCORE_ASSERT` 改用该宏。 |
| `Events/Event.h` | `EventType::##type` → `EventType::type`(去掉 `##`,MSVC 的 token-pasting,clang 上是多余且报错)。 |
| `ImGui/ImGuiLayer.h` | 成员函数定义去掉多余的类名限定:`ImGuiLayer::OnEvent` → `OnEvent`(+`override`);`ImGuiLayer::OnMouseButtonPressed` → `OnMouseButtonPressed`。 |

---

## 二、如何编译

### 前置:生成 makefile(premake5)

仓库里已有生成好的 makefile;若改动过 `premake5.lua`,需重新生成:

```bash
cd /Users/tal/CPP/OpenGL
premake5 gmake            # 生成 gmake 工具链的 makefile(旧名 gmake2 已重命名为 gmake)
```

> 顶层 `premake5.lua` 定义了两个独立 workspace:`OpenGL-Sandbox` 和 `OpenGL-Examples`。
> 生成后根目录会出现 `OpenGL-Examples.make` / `OpenGL-Sandbox.make`(workspace 编译入口),
> 以及各项目目录下的 `Makefile` / `*.make`。

### 编译命令

**用根目录的 workspace makefile 重编整个 workspace(含 OpenGL-Core):**

```bash
cd /Users/tal/CPP/OpenGL
make -f OpenGL-Examples.make      # 编 Examples workspace(GLFW + Glad + ImGui + OpenGL-Core + OpenGL-Examples)
make -f OpenGL-Sandbox.make       # 编 Sandbox workspace(同理)
```

⚠️ **重要坑**:`make -C OpenGL-Examples` 只重链 Examples,**不会重编 OpenGL-Core** ——
即使你改了 Core 的源码,`.a` 时间戳不更新,跑的还是旧 Core。改了 Core 源码后**必须**用
上面的 `make -f OpenGL-*.make` 重编整个 workspace。

### 产物位置

```
bin/Debug-macosx-x86_64/OpenGL-Core/libOpenGL-Core.a
bin/Debug-macosx-x86_64/OpenGL-Examples/OpenGL-Examples      ← 可执行文件
bin/Debug-macosx-x86_64/OpenGL-Sandbox/OpenGL-Sandbox
```

> premake 里 `architecture "x64"` 在 macOS 上产出 **x86_64** 二进制,Apple Silicon 上靠
> **Rosetta 2** 运行。(可选优化:改成 arm64 native,但不在本次适配范围内。)

---

## 三、如何运行

### 1. 装 Rosetta 2(x86_64 二进制必需,只装一次)

```bash
softwareupdate --install-rosetta --agree-to-license
```

### 2. 启动 Examples

⚠️ **必须从 `OpenGL-Examples/` 目录启动**,否则 exe 找不到 `assets/shaders/`(着色器路径
`"assets/shaders/test.vert.glsl"` 是相对当前工作目录的)。用绝对路径跑 exe 最稳:

```bash
cd /Users/tal/CPP/OpenGL/OpenGL-Examples && /Users/tal/CPP/OpenGL/bin/Debug-macosx-x86_64/OpenGL-Examples/OpenGL-Examples
```

或者(等价的相对路径写法,注意是 `../bin` 两个点):

```bash
cd /Users/tal/CPP/OpenGL/OpenGL-Examples
../bin/Debug-macosx-x86_64/OpenGL-Examples/OpenGL-Examples
```

正常的话:控制台打印 OpenGL 信息 + `[Diag] Shader=1 VAO=1 VB=1 IB=2`,窗口里出现彩色
四边形 + ImGui 的 "Controls" 面板。**关窗口或 Ctrl-C 退出。**

Sandbox 同理(从 `OpenGL-Sandbox/` 目录跑对应 exe)。

### 验证是否正常运行

跑起来后看进程状态(不是 `U` 就说明没楔死):

```bash
ps -o pid,stat,command -p $(pgrep -f OpenGL-Examples/OpenGL-Examples)
# 期望: STAT 为 S 或 SN(可中断睡眠,正常事件循环);若是 U 则已楔死,需重启
```

---

## 四、等待 commit 的内容(本次适配改动清单)

以下改动目前**均未提交**,可据此 review 后统一 commit。共 14 个已跟踪文件改动
(`git diff --stat` 约 +100/-15)。

### 源码改动

| 文件 | 改动说明 |
| --- | --- |
| `OpenGL-Core/premake5.lua` | `opengl32` 从公共 links 移入 Windows filter;新增 macOS filter(`GLCORE_PLATFORM_MACOS` + `GLFW_INCLUDE_NONE`)。 |
| `OpenGL-Examples/premake5.lua` | 新增 macOS filter:显式 `links { GLFW, Glad, ImGui }` + 框架 `linkoptions`(OpenGL/Cocoa/CoreVideo/IOKit)。 |
| `OpenGL-Sandbox/premake5.lua` | 同上。 |
| `OpenGL-Core/src/GLCore/Core/Core.h` | `__debugbreak()` → 分平台宏 `GLCORE_DEBUGBREAK()`(MSVC / clang)。 |
| `OpenGL-Core/src/GLCore/Events/Event.h` | `EventType::##type` → `EventType::type`(去 `##`)。 |
| `OpenGL-Core/src/GLCore/ImGui/ImGuiLayer.h` | 成员函数定义去类名限定 + `override`。 |
| `OpenGL-Core/src/GLCore/ImGui/ImGuiLayer.cpp` | 多视口 `#ifndef __APPLE__` 关闭。 |
| `OpenGL-Core/src/Platform/Windows/WindowsWindow.cpp` | 加 GL 3.3 core + `FORWARD_COMPAT`(macOS)hint。 |
| `OpenGL-Core/src/GLCore/Util/OpenGLDebug.cpp` | `EnableGLDebugging` 用 `#ifndef __APPLE__` 包裹,macOS 空操作(**修复窗口黑屏的关键改动**)。 |
| `OpenGL-Examples/assets/shaders/test.vert.glsl` | `#version 450 core` → `410 core`。 |
| `OpenGL-Examples/assets/shaders/test.frag.glsl` | `#version 450 core` → `410 core`。 |
| `OpenGL-Examples/src/ExampleLayer.cpp` | `glCreateVertexArrays/Buffers` → `glGenVertexArrays/Buffers`(DSA→3.0);加 `[Diag]` 诊断日志。 |

### 未跟踪的生成文件(建议加入 .gitignore,**不要 commit**)

`git status` 里带 `??` 的是 premake 生成的 makefile 产物:

```
OpenGL-Core/OpenGL-Core.make
OpenGL-Core/vendor/Glad/Glad.make
OpenGL-Examples.make
OpenGL-Examples/Makefile
OpenGL-Sandbox.make
OpenGL-Sandbox/Makefile
```

建议在仓库根 `.gitignore` 里加上,避免误提交:

```
*.make
Makefile
bin/
bin-int/
```

---

## 五、调试教训(macOS GL 驱动楔死)

- **楔死现象**:GL 调用把 Metal 驱动卡住后,进程进入 `U`(uninterruptible wait)状态,
  `kill -9` 无效,会越积越多卡死进程,**只能重启清掉**。
- **排查手段**:在可疑 GL 调用前后加 `fprintf(stderr, "[trace] ..."); fflush(stderr);`
  (stderr 无缓冲,卡死前最后一条即卡死位置),比 spdlog 的 `LOG_INFO` 更可靠
  (`LOG_INFO` 可能被缓冲、flush 不及时)。本次定位 `EnableGLDebugging` 楔死就是靠这个。
- **定位思路**:窗口出现但全黑 + 进程 `U` 状态 = 卡在 `OnAttach` 阶段(渲染主循环 `Run()`
  还没机会跑)。从 `OnAttach` 起逐句加 trace,看最后一条打在哪。
- **每次跑都会留进程**:GUI 程序不主动退出,调试时记得手动关窗口 / kill;若变成 `U`
  状态就只能重启,别反复硬跑堆积僵尸。
