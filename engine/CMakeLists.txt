# ---- 引擎根目录与第三方库路径定义 ----
# 设置引擎根目录（相对于项目根目录 SAMMI_ROOT_DIR）
set(ENGINE_ROOT_DIR "${SAMMI_ROOT_DIR}/engine")
# 设置第三方依赖库目录（位于引擎根目录下的 3rdparty 子目录）
set(THIRD_PARTY_DIR "${ENGINE_ROOT_DIR}/3rdparty")
# 设置引擎资源目录（绝对路径，可能存放引擎内置资源如纹理、模型等）
set(ENGINE_ASSET_DIR "/asset")

# ---- 配置文件目录定义 ----
# 部署环境配置目录（存放部署相关的配置文件，如发布版参数）
set(DEPLOY_CONFIG_DIR "configs/deployment")
# 开发环境配置目录（存放开发调试相关的配置文件，如调试参数）
set(DEVELOP_CONFIG_DIR "configs/development")

# ---- 物理调试渲染器开关（可选选项） ----
# 定义一个布尔型选项，用户可通过 -DENABLE_PHYSICS_DEBUG_RENDERER=ON 启用，默认关闭
option(ENABLE_PHYSICS_DEBUG_RENDERER "Enable Physics Debug Renderer" OFF)

# 仅在 Windows 平台支持物理调试渲染器（其他平台强制关闭）
if(NOT WIN32)
  # 如果用户在非 Windows 平台尝试启用该选项
  if(ENABLE_PHYSICS_DEBUG_RENDERER)
    message(WARNING "Disable Physics Debug Renderer")
    set(ENABLE_PHYSICS_DEBUG_RENDERER OFF CACHE BOOL "" FORCE)
  endif()
endif()

# 若启用了物理调试渲染器，设置 Jolt 物理引擎的资源目录（可能与 Jolt 物理引擎集成）
if(ENABLE_PHYSICS_DEBUG_RENDERER)
  set(JOLT_ASSET_DIR "/jolt-asset")
endif()

# ---- 编译器与平台特定配置 ----
# 若使用 MSVC 编译器（Windows 平台）
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    add_compile_options("/MP")  # 启用多进程编译（加速编译）
    # 设置 Visual Studio 启动项目为 SammiEditor（IDE 中双击打开时默认加载此项目）
    set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT SammiEditor)
endif()

# ---- Vulkan 图形 API 路径配置（跨平台） ----
# Vulkan 头文件目录（第三方 Vulkan SDK 的头文件路径）
set(vulkan_include ${THIRD_PARTY_DIR}/VulkanSDK/include)

# 根据不同操作系统设置 Vulkan 库文件、验证工具路径及相关编译定义
if(WIN32)
    # Windows 平台
    set(vulkan_lib ${THIRD_PARTY_DIR}/VulkanSDK/lib/Win32/vulkan-1.lib)
    set(glslangValidator_executable ${THIRD_PARTY_DIR}/VulkanSDK/bin/Win32/glslangValidator.exe)
    add_compile_definitions("SAMMI_VK_LAYER_PATH=${THIRD_PARTY_DIR}/VulkanSDK/bin/Win32")
elseif(UNIX)
    if(APPLE)
      set(vulkan_lib ${THIRD_PARTY_DIR}/VulkanSDK/lib/MacOS/libvulkan.1.dylib)
      set(glslangValidator_executable ${THIRD_PARTY_DIR}/VulkanSDK/bin/MacOS/glslangValidator)
      add_compile_definitions("SAMMI_VK_LAYER_PATH=${THIRD_PARTY_DIR}/VulkanSDK/bin/MacOS")
      add_compile_definitions("SAMMI_VK_ICD_FILENAMES=${THIRD_PARTY_DIR}/VulkanSDK/bin/MacOS/MoltenVK_icd.json")
    else()
      set(vulkan_lib ${THIRD_PARTY_DIR}/VulkanSDK/lib/Linux/libvulkan.so.1)
      set(glslangValidator_executable ${THIRD_PARTY_DIR}/VulkanSDK/bin/Linux/glslangValidator)
      add_compile_definitions("SAMMI_VK_LAYER_PATH=${THIRD_PARTY_DIR}/VulkanSDK/bin/Linux")
    endif()
else()
  message(FATAL_ERROR "Unknown Platform")
endif()

# ---- 着色器编译模块配置 ----
# 定义着色器编译目标的名称（用于后续依赖管理）
set(SHADER_COMPILE_TARGET SammiShaderCompile)
# 添加 shader 子目录（处理着色器编译逻辑，如生成 SPIR-V 中间代码）
add_subdirectory(shader)

# ---- 第三方库构建 ----
# 添加 3rdparty 子目录（构建项目依赖的第三方库，如 Vulkan SDK 工具链）
add_subdirectory(3rdparty)

# ---- 项目核心模块构建 ----
# 添加运行时模块（引擎核心功能，如渲染、物理、资源管理等）
add_subdirectory(source/runtime)
# 添加编辑器模块（编辑器界面、工具链等，依赖运行时模块）
add_subdirectory(source/editor)
# 添加元解析器模块（处理自定义资源格式的解析，如场景、材质的序列化）
add_subdirectory(source/meta_parser)
# 注释：测试模块（暂未启用，可通过取消注释启用）
#add_subdirectory(source/test)

# ---- 代码生成模块配置 ----
# 定义代码生成目标的名称（用于预编译或生成代码）
set(CODEGEN_TARGET "SammiPreCompile")
# 包含预编译脚本（可能用于生成反射代码、序列化代码或绑定接口）
include(source/precompile/precompile.cmake)
# 设置代码生成目标的文件夹分组（在 IDE 中归类到 "Engine" 文件夹下，方便管理）
set_target_properties("${CODEGEN_TARGET}" PROPERTIES FOLDER "Engine" )

# ---- 构建依赖管理 ----
# SammiRuntime 模块依赖代码生成目标（确保先生成代码，再编译运行时）
add_dependencies(SammiRuntime "${CODEGEN_TARGET}")
add_dependencies("${CODEGEN_TARGET}" "SammiParser")
