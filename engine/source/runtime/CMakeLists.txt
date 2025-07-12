
# ============================== 目标定义与基础配置 ==============================
# 定义当前模块的目标名称（运行时核心库）
set(TARGET_NAME SammiRuntime)

# 设置 C++ 标准为 C++17，并强制要求必须支持（否则构建失败）
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 指定 JSON 库（json11）的头文件路径（第三方库目录下的 json11 子目录）
#set(JSON_INCLUDE ${THIRD_PARTY_DIR}/json11)

# ------------------------------ 第三方库集成 ------------------------------
# 创建 json11 静态库（仅包含 json11.cpp 源文件）
#add_library(json11 ${JSON_INCLUDE}/json11.cpp)
# 在 IDE 中将该库归类到 "ThirdParty/json11" 文件夹（提升项目结构清晰度）
#set_target_properties(json11 PROPERTIES FOLDER "ThirdParty/json11")

# ------------------------------ 源文件收集与分组 ------------------------------
# 注意：使用 glob 递归收集文件不推荐（CMake 可能无法自动检测新增文件），修改文件时需手动更新
# 但此处仍用于快速收集当前目录及子目录下的所有头文件和源文件
file(GLOB_RECURSE HEADER_FILES "*.h")      # 所有头文件
file(GLOB_RECURSE SOURCE_FILES "*.cpp")    # 所有源文件

# 在 IDE（如 Visual Studio）中按目录结构分组文件（基于当前 CMakeLists.txt 所在目录）
#source_group(TREE "${ENGINE_ROOT_DIR}/schema" FILES ${SCHEMA_FILES})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${HEADER_FILES} ${SOURCE_FILES})


# if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/schema/)
#   file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/schema/" DESTINATION "${CMAKE_BINARY_DIR}/schema")
# endif()

# if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/asset/)
#   file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/asset/" DESTINATION "${CMAKE_BINARY_DIR}/asset")
# endif()

# ------------------------------ 构建库目标 ------------------------------
# 创建静态库目标（PiccoloRuntime），包含所有头文件和源文件
add_library(${TARGET_NAME} ${HEADER_FILES} ${SOURCE_FILES})

# 设置 C++ 标准（冗余，因前面已全局设置，但显式声明更清晰）
set_target_properties(${TARGET_NAME} PROPERTIES CXX_STANDARD 17)
# 在 IDE 中将目标归类到 "Engine" 文件夹（提升项目结构清晰度）
set_target_properties(${TARGET_NAME} PROPERTIES FOLDER "Engine")

# ------------------------------ 编译选项（MSVC 特定） ------------------------------
# 针对 MSVC 编译器添加严格编译选项：
# - /permissive-：强制严格符合 C++ 标准（禁止非标准扩展）
# - /WX：将警告视为错误（提升代码健壮性）
target_compile_options(${TARGET_NAME} PUBLIC "$<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/permissive->")
target_compile_options(${TARGET_NAME} PUBLIC "$<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/WX->")

# ------------------------------ 依赖链接 ------------------------------
# 公开依赖（依赖 PiccoloRuntime 的目标会自动继承这些依赖）：
target_link_libraries(${TARGET_NAME} PUBLIC 
#  spdlog::spdlog  # 日志库（公开，因运行时功能可能依赖日志）
#  glfw            # 窗口与输入管理（公开，运行时可能需要窗口交互）
#  imgui           # GUI 库（公开，运行时可能需要基础 UI 支持）
#  Jolt            # 物理引擎（公开，运行时核心物理功能）
#  lua_static      # Lua 解释器（公开，运行时可能执行 Lua 脚本）
#  sol2            # Lua 绑定库（公开，连接 C++ 与 Lua）
  ${vulkan_lib}   # Vulkan 图形 API 库（公开，运行时图形渲染核心）
)

# 私有依赖（仅当前库内部使用，不传递给依赖它的目标）：
#target_link_libraries(${TARGET_NAME} PRIVATE 
#  tinyobjloader   # 模型加载库（私有，仅运行时内部解析模型）
#  stb             # 轻量级图像/字体库（私有，仅运行时内部资源加载）
#  TestFramework   # 测试框架（私有，仅调试/测试场景使用）
#  d3d12.lib       # DirectX 12 库（私有，仅物理调试渲染器使用）
#  shcore.lib      # Windows 系统库（私有，仅物理调试渲染器使用）
#)


if(ENABLE_PHYSICS_DEBUG_RENDERER)
  add_compile_definitions(ENABLE_PHYSICS_DEBUG_RENDERER)
  target_link_libraries(${TARGET_NAME} PUBLIC TestFramework d3d12.lib shcore.lib)
endif()

# ------------------------------ 头文件包含路径 ------------------------------
# 公开包含路径（依赖 PiccoloRuntime 的目标可直接访问这些路径）：
#target_include_directories(
  #${TARGET_NAME}
  # 构建时路径：Vulkan SDK 头文件（仅在构建阶段有效）
  #$<BUILD_INTERFACE:${vulkan_include}>
  # 构建时路径：生成的着色器代码目录（运行时需要反射着色器结构）
#  $<BUILD_INTERFACE:${ENGINE_ROOT_DIR}/shader/generated/cpp>
  # 构建时路径：引擎根源码目录（运行时可能需要访问核心接口）
  #$<BUILD_INTERFACE:${ENGINE_ROOT_DIR}/source>
  # 构建时路径：运行时源码目录（当前库的核心实现）
  #$<BUILD_INTERFACE:${ENGINE_ROOT_DIR}/source/runtime>
  # 构建时路径：运行时渲染模块头文件（当前库的子模块）
#  $<BUILD_INTERFACE:${ENGINE_ROOT_DIR}/source/runtime/function/render/include>
  # 安装时路径：安装后其他项目可通过 <PREFIX>/include/PiccoloRuntime-<VERSION> 访问
#  $<INSTALL_INTERFACE:include/${TARGET_NAME}-${PROJECT_VERSION}>
  # 构建时路径：JSON 库头文件（私有依赖，但当前库需要直接包含）
#  $<BUILD_INTERFACE:${JSON_INCLUDE}>
  # 构建时路径：Vulkan 内存分配器头文件（私有依赖）
  #$<BUILD_INTERFACE:${THIRD_PARTY_DIR}/vulkanmemoryallocator/include>
#)

# ------------------------------ 预编译与依赖管理 ------------------------------
# 声明当前库依赖着色器编译目标（确保着色器在运行时库构建前完成生成）
#add_dependencies(${TARGET_NAME} ${SHADER_COMPILE_TARGET})

# ------------------------------ 安装接口配置 ------------------------------
# （用于通过 `find_package(PiccoloRuntime)` 其他项目使用）
# 定义版本头文件的输出路径（与常规头文件路径一致，便于兼容）
#string(TOLOWER ${TARGET_NAME}/version.h VERSION_HEADER_LOCATION)

# ------------------------------ 预编译相关（注释部分） ------------------------------
# （以下为注释掉的预编译配置，可能用于生成代码或反射）
# # 设置预编译阶段需要的全局变量（当前库的头文件列表）
#set(PICCOLO_RUNTIME_HEADS "${HEADER_FILES}" PARENT_SCOPE)
