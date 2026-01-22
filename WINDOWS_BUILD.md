# Windows 构建指南

本文档详细说明如何在 Windows 平台上编译 GB28181Client 项目。

## 环境要求

- Windows 10/11 (64位)
- Visual Studio 2019/2022 或 MinGW-w64
- CMake 3.16 或更高版本
- Git

## 依赖库安装

### 1. 安装 Qt 6

**方法一：在线安装器（推荐）**

1. 下载 Qt 在线安装器：https://www.qt.io/download-qt-installer
2. 运行安装器，选择组件：
   - Qt 6.5.x (或更高版本)
   - MSVC 2019 64-bit 或 MinGW 64-bit
   - Qt Creator (可选)
3. 安装路径示例：`C:\Qt\6.5.0\msvc2019_64`

**方法二：离线安装包**

从 Qt 官网下载离线安装包：https://www.qt.io/offline-installers

### 2. 安装 FFmpeg

**方法一：使用预编译版本（推荐）**

1. 下载 FFmpeg 预编译包：
   - https://github.com/BtbN/FFmpeg-Builds/releases
   - 选择 `ffmpeg-master-latest-win64-gpl-shared.zip`

2. 解压到目录，例如：`C:\ffmpeg`

3. 目录结构应为：
   ```
   C:\ffmpeg\
   ├── bin\      (DLL 文件)
   ├── include\  (头文件)
   └── lib\      (导入库)
   ```

**方法二：使用 vcpkg**

```powershell
vcpkg install ffmpeg:x64-windows
```

### 3. 安装 eXosip2

**方法一：使用预编译版本**

由于 eXosip2 的 Windows 预编译版本较少，推荐使用 vcpkg 或自行编译。

**方法二：使用 vcpkg（推荐）**

```powershell
# 安装 vcpkg
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat

# 安装 eXosip2
.\vcpkg install exosip:x64-windows
```

**方法三：从源码编译**

1. 下载源码：
   ```powershell
   git clone https://github.com/cisco/libosip2.git
   git clone https://github.com/cisco/libexosip2.git
   ```

2. 使用 CMake 编译 libosip2：
   ```powershell
   cd libosip2
   mkdir build && cd build
   cmake .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_INSTALL_PREFIX=C:\exosip2
   cmake --build . --config Release
   cmake --install .
   ```

3. 编译 libexosip2：
   ```powershell
   cd libexosip2
   mkdir build && cd build
   cmake .. -G "Visual Studio 17 2022" -A x64 ^
     -DCMAKE_INSTALL_PREFIX=C:\exosip2 ^
     -DOSIP2_INCLUDE_DIR=C:\exosip2\include ^
     -DOSIP2_LIBRARY=C:\exosip2\lib\osip2.lib
   cmake --build . --config Release
   cmake --install .
   ```

### 4. 安装 CMake

从官网下载安装：https://cmake.org/download/

或使用 Chocolatey：
```powershell
choco install cmake
```

## 编译项目

### 1. 克隆项目

```powershell
cd C:\Users\YourName\Desktop
git clone <your-repo-url> GB28181Client
cd GB28181Client
```

### 2. 配置环境变量

创建 `build.bat` 脚本：

```batch
@echo off
REM 设置 Qt 路径
set Qt6_DIR=C:\Qt\6.5.0\msvc2019_64

REM 设置 FFmpeg 路径
set FFMPEG_DIR=C:\ffmpeg

REM 设置 eXosip2 路径
set EXOSIP2_DIR=C:\exosip2

REM 或者使用 vcpkg
REM set CMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake

REM 创建构建目录
if not exist build mkdir build
cd build

REM 配置项目
cmake .. -G "Visual Studio 17 2022" -A x64 ^
  -DCMAKE_PREFIX_PATH=%Qt6_DIR% ^
  -DFFMPEG_DIR=%FFMPEG_DIR% ^
  -DEXOSIP2_DIR=%EXOSIP2_DIR%

REM 编译项目
cmake --build . --config Release

pause
```

### 3. 运行构建脚本

```powershell
.\build.bat
```

### 4. 复制依赖 DLL

编译完成后，需要将依赖的 DLL 文件复制到可执行文件目录：

```powershell
cd build\Release

REM 复制 Qt DLL
copy C:\Qt\6.5.0\msvc2019_64\bin\Qt6Core.dll .
copy C:\Qt\6.5.0\msvc2019_64\bin\Qt6Gui.dll .
copy C:\Qt\6.5.0\msvc2019_64\bin\Qt6Widgets.dll .
copy C:\Qt\6.5.0\msvc2019_64\bin\Qt6Network.dll .

REM 复制 Qt 平台插件
mkdir platforms
copy C:\Qt\6.5.0\msvc2019_64\plugins\platforms\qwindows.dll platforms\

REM 复制 FFmpeg DLL
copy C:\ffmpeg\bin\*.dll .

REM 复制 eXosip2 DLL
copy C:\exosip2\bin\*.dll .
```

或使用 Qt 的 `windeployqt` 工具：

```powershell
C:\Qt\6.5.0\msvc2019_64\bin\windeployqt.exe GB28181Client.exe
```

## 使用 Qt Creator 构建

1. 打开 Qt Creator
2. 选择 "文件" -> "打开文件或项目"
3. 选择 `CMakeLists.txt`
4. 配置构建套件（Kit）
5. 点击"配置项目"
6. 在"项目"面板中设置 CMake 参数：
   ```
   -DFFMPEG_DIR=C:/ffmpeg
   -DEXOSIP2_DIR=C:/exosip2
   ```
7. 点击"构建"按钮

## 常见问题

### 1. 找不到 Qt6

**解决方法**：
```powershell
set CMAKE_PREFIX_PATH=C:\Qt\6.5.0\msvc2019_64
```

### 2. 找不到 FFmpeg

**解决方法**：
在 CMakeLists.txt 中明确指定路径：
```cmake
set(FFMPEG_DIR "C:/ffmpeg" CACHE PATH "FFmpeg directory")
```

### 3. 链接错误

**解决方法**：
确保所有库都是相同的架构（x64）和编译器（MSVC 或 MinGW）。

### 4. 运行时缺少 DLL

**解决方法**：
使用 Dependency Walker 或 Dependencies 工具检查缺少的 DLL：
- https://github.com/lucasg/Dependencies

### 5. eXosip2 编译失败

**解决方法**：
使用 vcpkg 安装：
```powershell
vcpkg install exosip:x64-windows
```

然后在 CMake 中使用：
```powershell
cmake .. -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake
```

## 打包发布

### 创建安装包

使用 NSIS 或 Inno Setup 创建安装程序。

**示例：使用 windeployqt**

```powershell
cd build\Release
mkdir deploy
copy GB28181Client.exe deploy\
cd deploy
C:\Qt\6.5.0\msvc2019_64\bin\windeployqt.exe GB28181Client.exe

REM 复制其他依赖
copy C:\ffmpeg\bin\*.dll .
copy C:\exosip2\bin\*.dll .
```

### 创建便携版

将 `deploy` 目录打包为 ZIP 文件即可。

## 调试技巧

### 1. 启用详细日志

在代码中添加：
```cpp
qSetMessagePattern("[%{time yyyy-MM-dd hh:mm:ss.zzz}] [%{type}] %{message}");
```

### 2. 使用 Visual Studio 调试器

在 Visual Studio 中打开 `.sln` 文件，设置断点进行调试。

### 3. 检查网络通信

使用 Wireshark 抓包分析 SIP 和 RTP 流量。

## 性能优化

### 1. 启用编译器优化

在 CMakeLists.txt 中：
```cmake
if(MSVC)
    add_compile_options(/O2 /GL)
    add_link_options(/LTCG)
endif()
```

### 2. 使用 Release 模式

```powershell
cmake --build . --config Release
```

## 参考资源

- [Qt for Windows](https://doc.qt.io/qt-6/windows.html)
- [FFmpeg Windows 编译](https://trac.ffmpeg.org/wiki/CompilationGuide/MSVC)
- [vcpkg 使用指南](https://vcpkg.io/en/getting-started.html)
