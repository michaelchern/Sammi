
# 设置目标名称
set(TARGET_NAME SammiRuntime)

# 设置C++标准为17，且必须支持17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置JSON库的包含路径
set(JSON_INCLUDE ${THIRD_PARTY_DIR}/json11)

# 添加JSON11库目标
add_library(json11 ${JSON_INCLUDE}/json11.cpp)
set_target_properties(json11 PROPERTIES FOLDER "ThirdParty/json11")

# ---- 添加源文件 ----
# 警告：使用通配符收集源文件被视为不良实践（因为CMake生成器不会自动检测新文件）
# 如果文件结构变更，请显式列出文件而非使用通配符

# 递归收集所有头文件和源文件
file(GLOB_RECURSE HEADER_FILES "*.h")
file(GLOB_RECURSE SOURCE_FILES "*.cpp")

# 为头文件和源文件创建IDE分组
#source_group(TREE "${ENGINE_ROOT_DIR}/schema" FILES ${SCHEMA_FILES})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${HEADER_FILES} ${SOURCE_FILES})

# 复制资源文件到构建目录
#if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/schema/)
#  file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/schema/" DESTINATION "${CMAKE_BINARY_DIR}/schema")
#endif()

#if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/asset/)
#  file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/asset/" DESTINATION "${CMAKE_BINARY_DIR}/asset")
#endif()

# ---- 创建库目标 ----
# 创建名为SammiRuntime的库，包含所有头文件和源文件
add_library(${TARGET_NAME} ${HEADER_FILES} ${SOURCE_FILES})

# 设置目标属性：强制使用C++17标准
set_target_properties(${TARGET_NAME} PROPERTIES CXX_STANDARD 17)
# 设置目标在IDE中的组织位置
set_target_properties(${TARGET_NAME} PROPERTIES FOLDER "Engine")

# 跨平台兼容性设置：在MSVC上启用标准符合性检查
target_compile_options(${TARGET_NAME} PUBLIC "$<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/permissive->")
# 关闭MSVC上的警告作为错误（根据需求可修改为/WX开启）
target_compile_options(${TARGET_NAME} PUBLIC "$<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/WX->")

# 链接依赖库
target_link_libraries(${TARGET_NAME} PUBLIC spdlog::spdlog)              # 日志系统
#target_link_libraries(${TARGET_NAME} PRIVATE tinyobjloader stb)          # OBJ加载器和图像处理库
#target_link_libraries(${TARGET_NAME} PUBLIC glfw)                        # 跨平台窗口库
#target_link_libraries(${TARGET_NAME} PUBLIC imgui)                       # 即时GUI渲染
#target_link_libraries(${TARGET_NAME} PUBLIC Jolt)                        # 物理引擎
#target_link_libraries(${TARGET_NAME} PUBLIC lua_static sol2)             # Lua脚本支持
#target_link_libraries(${TARGET_NAME} PUBLIC ${vulkan_lib})               # Vulkan图形API
target_link_libraries(${TARGET_NAME} PRIVATE $<BUILD_INTERFACE:json11>)  # 内部JSON库

# 物理调试渲染器的可选支持
if(ENABLE_PHYSICS_DEBUG_RENDERER)
  # 启用编译预处理宏
  add_compile_definitions(ENABLE_PHYSICS_DEBUG_RENDERER)
  # 链接调试渲染所需库
  target_link_libraries(${TARGET_NAME} PUBLIC TestFramework d3d12.lib shcore.lib)
endif()

# Vulkan包含目录（构建阶段）
target_include_directories(
  ${TARGET_NAME}
  PUBLIC $<BUILD_INTERFACE:${vulkan_include}>)

# 着色器编译依赖管理
#add_dependencies(${TARGET_NAME} ${SHADER_COMPILE_TARGET})
# 着色器生成的CPP头文件路径
target_include_directories(
  ${TARGET_NAME}
  PUBLIC $<BUILD_INTERFACE:${ENGINE_ROOT_DIR}/shader/generated/cpp>)

# 主要包含目录配置（构建与安装）
target_include_directories(
  ${TARGET_NAME} 
  PUBLIC $<BUILD_INTERFACE:${ENGINE_ROOT_DIR}/source>
  $<INSTALL_INTERFACE:include/${TARGET_NAME}-${PROJECT_VERSION}>
)

# JSON库包含目录
target_include_directories(
  ${TARGET_NAME} 
  PUBLIC $<BUILD_INTERFACE:${JSON_INCLUDE}>
)

# Vulkan内存分配器包含目录
target_include_directories(
  ${TARGET_NAME} 
  PUBLIC $<BUILD_INTERFACE:${THIRD_PARTY_DIR}/vulkanmemoryallocator/include>
)

# 运行时核心包含目录
target_include_directories(
  ${TARGET_NAME} 
  PUBLIC $<BUILD_INTERFACE:${ENGINE_ROOT_DIR}/source/runtime>
  $<INSTALL_INTERFACE:include/${TARGET_NAME}-${PROJECT_VERSION}>
)

# 渲染系统专用包含目录
target_include_directories(
  ${TARGET_NAME} 
  PUBLIC $<BUILD_INTERFACE:${ENGINE_ROOT_DIR}/source/runtime/function/render/include>
  $<INSTALL_INTERFACE:include/${TARGET_NAME}-${PROJECT_VERSION}>
)

# ---- 安装配置 ----
# 以下为创建可安装目标的基础准备，实际安装命令需要另行添加

# ---- Create an installable target ----
# this allows users to install and find the library via `find_package()`.

# the location where the project's version header will be placed should match the project's regular
# header paths
string(TOLOWER ${TARGET_NAME}/version.h VERSION_HEADER_LOCATION)


# 预编译头设置（需结合实际的预编译头实现）
# 将头文件列表导出到父作用域（供预编译脚本使用）
set(SAMMI_RUNTIME_HEADS "${HEADER_FILES}" PARENT_SCOPE)