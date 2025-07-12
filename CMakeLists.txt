# 指定 CMake 的最低版本要求为 3.19，若版本过低则报错终止
# 这是为了确保项目使用的 CMake 特性（如某些命令或模块）能被正确支持
cmake_minimum_required(VERSION 3.19 FATAL_ERROR)

project(Sammi VERSION 0.1.0)

# 设置 C++ 标准为 C++17（ISO C++17 标准）
# 编译器需要支持 C++17 特性来编译项目代码
set(CMAKE_CXX_STANDARD 17)

# 强制要求使用指定的 C++ 标准（不允许编译器降级到更低版本）
# 若编译器不支持 C++17，CMake 配置阶段会报错
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 禁止构建共享库（Dynamic Library），默认构建静态库（Static Library）
# BUILD_SHARED_LIBS 是 CMake 内置变量，控制库的构建类型
set(BUILD_SHARED_LIBS OFF)

# 包含 CMake 的依赖选项模块，用于处理条件依赖的选项（如后续可能出现的可选功能开关）
include(CMakeDependentOption)

# -------------------- 源目录与构建目录隔离保护 --------------------
# 检查源目录（PROJECT_SOURCE_DIR）是否与构建目录（PROJECT_BINARY_DIR）相同
# 若相同，说明用户在源目录直接运行了 CMake（即 "in-source build"）
if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
  message(
    FATAL_ERROR
      "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there."
  )
endif()

# 定义项目根目录为当前源目录路径（即 CMakeLists.txt 所在目录）
# 后续路径设置可基于此变量，提高可移植性
set(SAMMI_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

# 设置安装前缀（Install Prefix）为项目根目录下的 bin 目录
# 安装操作（如 `make install`）会将可执行文件、库等安装到此处
# 默认情况下，CMAKE_INSTALL_PREFIX 可能指向系统目录（如 /usr/local），此处显式指定到项目本地
set(CMAKE_INSTALL_PREFIX "${SAMMI_ROOT_DIR}/bin")

# 定义二进制文件根目录为安装前缀路径（即与 CMAKE_INSTALL_PREFIX 一致）
# 可能用于后续安装步骤中指定可执行文件、库等的输出路径
set(BINARY_ROOT_DIR "${CMAKE_INSTALL_PREFIX}/")

# 将 engine 子目录添加到构建系统中，递归处理其中的 CMakeLists.txt
# engine 目录应包含项目核心引擎的代码和构建规则
add_subdirectory(engine)