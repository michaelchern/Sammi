# 指定 CMake 的最低版本要求为 3.19，若版本过低则报错终止配置过程
cmake_minimum_required(VERSION 3.19 FATAL_ERROR)

project(Sammi VERSION 0.1.0)

# 设置 C++ 标准为 C++17（告诉编译器使用 C++17 特性编译代码）
set(CMAKE_CXX_STANDARD 17)
# 强制要求必须支持 C++17 标准（若编译器不支持则配置失败）
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# 关闭共享库构建（默认构建静态库，即生成 .a 或 .lib 文件而非 .so/.dll）
set(BUILD_SHARED_LIBS OFF)

# 包含 CMake 内置模块 CMakeDependentOption（用于定义有依赖关系的可选选项）
include(CMakeDependentOption)

# ---- 源目录与构建目录隔离检查 ----
# 若源目录（PROJECT_SOURCE_DIR）与构建目录（PROJECT_BINARY_DIR）相同（即尝试在源码目录直接运行 cmake）
if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
  message(
    FATAL_ERROR
      "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there."
  )
endif()

# 定义项目根目录变量（指向当前 CMakeLists.txt 所在的源码目录）
set(SAMMI_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
# 设置安装前缀路径（最终安装的二进制文件、库等会存放在此目录下）
# 此处设置为项目根目录下的 bin 目录（例如：./Sammi/bin）
set(CMAKE_INSTALL_PREFIX "${SAMMI_ROOT_DIR}/bin")
# 定义二进制文件根目录变量（此处直接指向安装前缀目录，可能与安装路径一致）
set(BINARY_ROOT_DIR "${CMAKE_INSTALL_PREFIX}/")

# 将 engine 子目录添加到构建系统中（递归处理 engine 目录下的 CMakeLists.txt）
# 通常 engine 是项目的核心模块，包含引擎相关的源码、头文件和构建规则
add_subdirectory(engine)