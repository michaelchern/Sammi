# 设置CMake最低版本要求为3.19，不满足则报错退出
cmake_minimum_required(VERSION 3.19 FATAL_ERROR)

# 定义项目名称为Sammi，版本号为0.1.0
project(Sammi VERSION 0.1.0)

# 设置C++标准为C++17
set(CMAKE_CXX_STANDARD 17)
# 要求必须支持指定的C++标准
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# 默认构建静态库(.a/.lib)而不是动态库(.so/.dll)
set(BUILD_SHARED_LIBS OFF)

# 包含CMake的条件选项模块（用于创建依赖关系的选项）
include(CMakeDependentOption)

# ==== 源码构建防护 ====
# 检查是否在源码目录内进行构建
if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
  message(
    FATAL_ERROR
      "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there."
  )
endif()

# 设置项目根目录为当前源码目录
set(SAMMI_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
# 配置安装路径为项目根目录下的bin目录
set(CMAKE_INSTALL_PREFIX "${SAMMI_ROOT_DIR}/bin")
# 设置二进制文件根目录路径变量
set(BINARY_ROOT_DIR "${CMAKE_INSTALL_PREFIX}/")

# 添加子目录engine（递归处理其中的CMakeLists.txt）
# 通常是核心引擎代码的编译入口
add_subdirectory(engine)