# 使用较旧的 CMP0074 策略行为（处理 NEW 策略引入前的 find_package 逻辑）
#cmake_policy(SET CMP0074 OLD)

# 定义目标名称变量
set(TARGET_NAME SammiParser)

#set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/../CMake")

# 设置全局属性：在支持 IDE（如 Visual Studio）中启用文件夹分组功能
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# 递归收集当前目录及子目录下所有 .h 头文件和 .cpp 源文件
file(GLOB_RECURSE HEADERS "*.h")
file(GLOB_RECURSE SOURCES "*.cpp")

# 为 IDE 创建源代码分组（保持目录结构）
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${HEADERS} ${SOURCES})

# 设置 LLVM 包含路径（项目目录 + 第三方库）
set(LLVM_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd_party/LLVM/include
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd_party/mustache
    ${CMAKE_CURRENT_SOURCE_DIR})

# 设置输出目录（Release/Debug 版本都输出到引擎的 bin 目录）
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${ENGINE_ROOT_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${ENGINE_ROOT_DIR}/bin)

# 添加头文件包含路径（LLVM + 项目内部）
include_directories(${LLVM_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/parser)

# 创建可执行目标（包含头文件以便在 IDE 中显示）
add_executable(${TARGET_NAME} ${HEADERS} ${SOURCES})

#set_target_properties(meta_parser PROPERTIES FOLDER "generator" ) 

# 配置目标属性：C++17 标准，在 IDE 中归入 "Tools" 文件夹
set_target_properties(${TARGET_NAME} PROPERTIES CXX_STANDARD 17)
set_target_properties(${TARGET_NAME} PROPERTIES FOLDER "Tools")

# 平台特定配置
if (CMAKE_HOST_WIN32)
    # Windows 配置
    set(LLVM_LIBRARY_DIR  ${CMAKE_CURRENT_SOURCE_DIR}/3rd_party/LLVM/lib/x64)
    set(LLVM_SHARED_LIBRARY_DIR  ${CMAKE_CURRENT_SOURCE_DIR}/3rd_party/LLVM/bin/x64)
    set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} /O2 /Ob2")  # 调试版优化设置
    target_link_libraries(${TARGET_NAME}  ${LLVM_LIBRARY_DIR}/libclang.lib)  # 静态链接 LLVM
elseif(${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Linux")
    # Linux 配置
    set(LLVM_LIBRARY_DIR  ${CMAKE_CURRENT_SOURCE_DIR}/3rd_party/LLVM/lib/Linux)
    set(LLVM_SHARED_LIBRARY_DIR  ${CMAKE_CURRENT_SOURCE_DIR}/3rd_party/LLVM/bin/Linux)
    set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O3")
    target_link_libraries(${TARGET_NAME}  ${LLVM_SHARED_LIBRARY_DIR}/libclang.so.12)
elseif(${CMAKE_HOST_SYSTEM_PROCESSOR} STREQUAL "arm64") 
    # macOS ARM64 配置（使用系统工具链）
    set(LLVM_LIBRARY_DIR  ${CMAKE_CURRENT_SOURCE_DIR}/3rd_party/LLVM/lib/macOS)
    set(LLVM_SHARED_LIBRARY_DIR  ${OSX_DEVELOPER_ROOT}/Toolchains/XcodeDefault.xctoolchain/usr/lib)
    set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O3")
    target_link_libraries(${TARGET_NAME}  ${LLVM_SHARED_LIBRARY_DIR}/libclang.dylib)    
else()
    # 其他 macOS 配置
    set(LLVM_LIBRARY_DIR  ${CMAKE_CURRENT_SOURCE_DIR}/3rd_party/LLVM/lib/macOS)
    set(LLVM_SHARED_LIBRARY_DIR  ${CMAKE_CURRENT_SOURCE_DIR}/3rd_party/LLVM/bin/macOS)
    set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O3")
    target_link_libraries(${TARGET_NAME}  ${LLVM_SHARED_LIBRARY_DIR}/libclang.dylib)
endif()

#link_directories(${LLVM_LIBRARY_DIR})

# 启用 TinyXML 的 STL 支持
add_definitions(-DTIXML_USE_STL)

# 后构建步骤：复制运行时依赖库到输出目录
add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${LLVM_SHARED_LIBRARY_DIR}"       # 源目录（含动态库）
        $<TARGET_FILE_DIR:${TARGET_NAME}>  # 目标目录（可执行文件所在路径）
)

