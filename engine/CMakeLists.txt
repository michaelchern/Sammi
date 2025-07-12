# ============================== 基础路径配置 ==============================
# 设置引擎根目录（相对于 SAMMI_ROOT_DIR）
set(ENGINE_ROOT_DIR "${SAMMI_ROOT_DIR}/engine")
# 设置第三方库目录（引擎根目录下的 3rdparty 子目录）
set(THIRD_PARTY_DIR "${ENGINE_ROOT_DIR}/3rdparty")
# 引擎资源目录（绝对路径，可能后续会通过外部变量覆盖）
set(ENGINE_ASSET_DIR "/asset")

# 部署配置目录（存放部署相关配置文件）
#set(DEPLOY_CONFIG_DIR "configs/deployment")
# 开发配置目录（存放开发阶段配置文件）
#set(DEVELOP_CONFIG_DIR "configs/development")

# ============================== 物理调试渲染器选项 ==============================
# 定义选项：是否启用物理调试渲染器（默认关闭）
#option(ENABLE_PHYSICS_DEBUG_RENDERER "Enable Physics Debug Renderer" OFF)

# 跨平台限制：仅支持 Windows 平台的物理调试渲染器
if(NOT WIN32)
  # 如果当前平台不是 Windows，但用户尝试启用物理调试渲染器
  if(ENABLE_PHYSICS_DEBUG_RENDERER)
    # 输出警告信息，提示用户物理调试渲染器仅支持 Windows 平台
    message(WARNING "Disable Physics Debug Renderer! Only Windows platform supports it.")
    # 强制关闭物理调试渲染器选项
    set(ENABLE_PHYSICS_DEBUG_RENDERER OFF CACHE BOOL "" FORCE)
  endif()
endif()

# 如果启用了物理调试渲染器，设置 Jolt 物理引擎的资源目录
if(ENABLE_PHYSICS_DEBUG_RENDERER)
  set(JOLT_ASSET_DIR "/jolt-asset")
endif()

# ============================== 编译器与平台特定配置 ==============================
# 如果使用 MSVC 编译器（Windows 平台常见编译器）
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    # 设置 MSVC 编译选项，启用多线程编译
    # "/MP" 选项可以加速编译过程，特别是在多核处理器上
    # 但可能会增加内存使用量
    add_compile_options("/MP")
    # 设置 Visual Studio 启动项目（打开解决方案时默认加载的项目）
    set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT SammiEditor)
endif()

# ============================== Vulkan 图形 API 配置 ==============================
# Vulkan 头文件路径（第三方 Vulkan SDK 的 include 目录）
set(vulkan_include ${THIRD_PARTY_DIR}/VulkanSDK/include)

if(WIN32)
    # Vulkan 库文件路径（Windows 32位）
    set(vulkan_lib ${THIRD_PARTY_DIR}/VulkanSDK/lib/Win32/vulkan-1.lib)
    # GLSL 着色器编译器可执行文件路径（用于验证和编译 GLSL 代码）
    set(glslangValidator_executable ${THIRD_PARTY_DIR}/VulkanSDK/bin/Win32/glslangValidator.exe)
    # 定义预编译宏：指定 Vulkan 层的路径（运行时加载验证层用）
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

# ============================== 着色器编译与子项目集成 ==============================
# 定义着色器编译目标的名称
#set(SHADER_COMPILE_TARGET SammiShaderCompile)
# 添加 shader 子目录（该目录包含着色器编译的 CMake 脚本）
#add_subdirectory(shader)

# 添加第三方库子目录（构建第三方依赖，如 Vulkan SDK 封装库等）
#add_subdirectory(3rdparty)

# 添加核心运行时子目录（引擎核心功能实现）
add_subdirectory(source/runtime)
# 添加编辑器子目录（编辑器功能，依赖运行时）
add_subdirectory(source/editor)
# 添加元解析器子目录（处理资源元数据解析）
#add_subdirectory(source/meta_parser)
# 注释掉测试子目录（可能暂时不参与构建或集成测试）
#add_subdirectory(source/test)

# ============================== 代码生成与依赖管理 ==============================
# 定义代码生成目标的名称
#set(CODEGEN_TARGET "SammiPreCompile")
# 包含预编译脚本（该脚本负责生成代码，如反射、序列化等）
#include(source/precompile/precompile.cmake)
# 设置代码生成目标的文件夹分类（在 IDE 中组织显示）
#set_target_properties("${CODEGEN_TARGET}" PROPERTIES FOLDER "Engine" )

# 定义依赖关系：
# - SammiRuntime（运行时库）依赖代码生成目标（需先生成代码再编译运行时）
#add_dependencies(SammiRuntime "${CODEGEN_TARGET}")
# - 代码生成目标依赖 SammiParser（需先编译解析器再生成代码）
#add_dependencies("${CODEGEN_TARGET}" "SammiParser")
