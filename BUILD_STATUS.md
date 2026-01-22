# 编译指南 - 当前系统

由于编译这个项目需要以下依赖库：
- Qt 6.x
- FFmpeg 4.x/5.x
- eXosip2 5.x
- CMake 3.16+
- Visual Studio 2019/2022

这些依赖库的安装和配置比较复杂，我为你提供了几个方案：

## 方案一：使用 vcpkg（推荐，但需要时间）

vcpkg 可以自动下载和编译所有依赖库，但首次安装可能需要 1-2 小时。

### 步骤：

1. **安装依赖**（首次运行，约 1-2 小时）
   ```cmd
   install_deps_vcpkg.bat
   ```

2. **编译项目**
   ```cmd
   build_vcpkg.bat
   ```

## 方案二：手动安装依赖（快速，但需要手动配置）

### 1. 安装 CMake
下载：https://cmake.org/download/
安装后添加到 PATH

### 2. 安装 Qt 6
下载：https://www.qt.io/download-qt-installer
选择 MSVC 2019 64-bit 组件

### 3. 下载 FFmpeg 预编译版
下载：https://github.com/BtbN/FFmpeg-Builds/releases
选择：ffmpeg-master-latest-win64-gpl-shared.zip
解压到：C:\ffmpeg

### 4. 安装 eXosip2
使用 vcpkg 安装：
```cmd
git clone https://github.com/Microsoft/vcpkg.git C:\vcpkg
cd C:\vcpkg
bootstrap-vcpkg.bat
vcpkg install exosip:x64-windows
```

### 5. 修改 build.bat
编辑 build.bat，设置正确的路径：
```batch
set Qt6_DIR=C:\Qt\6.5.0\msvc2019_64
set FFMPEG_DIR=C:\ffmpeg
set EXOSIP2_DIR=C:\vcpkg\installed\x64-windows
```

### 6. 编译
```cmd
build.bat
```

## 方案三：使用预编译的演示版本（最快）

由于完整编译需要较多依赖，我可以为你创建一个简化的演示版本，只包含核心功能，不需要复杂的依赖。

这个版本将：
- 使用纯 C++ 实现 SIP 协议（不依赖 eXosip2）
- 使用 Windows Media Foundation 解码（不依赖 FFmpeg）
- 简化的 Qt 界面

你希望使用哪个方案？

## 当前系统状态

- ✅ Windows 10 (MINGW64)
- ❌ CMake 未安装
- ❌ Qt 未安装
- ❌ FFmpeg 未安装
- ❌ eXosip2 未安装

## 建议

如果你想快速看到效果，我建议：

1. **先安装 CMake**（5分钟）
   - 下载：https://cmake.org/download/
   - 安装时选择"Add CMake to system PATH"

2. **使用 vcpkg 安装其他依赖**（1-2小时，自动化）
   - 运行 `install_deps_vcpkg.bat`
   - 等待安装完成
   - 运行 `build_vcpkg.bat`

或者，如果你已经有 Qt、FFmpeg 等库的安装路径，请告诉我，我可以直接配置编译。
