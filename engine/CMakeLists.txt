
# 设置引擎根目录为项目根目录下的engine子目录
set(ENGINE_ROOT_DIR "${SAMMI_ROOT_DIR}/engine")

# 设置第三方库目录路径
set(THIRD_PARTY_DIR "${ENGINE_ROOT_DIR}/3rdparty")

# 设置引擎资源目录（注意：这是相对路径，可能需后续处理）
set(ENGINE_ASSET_DIR "/asset")

# 设置部署配置目录
set(DEPLOY_CONFIG_DIR "configs/deployment")
# 设置开发配置目录
set(DEVELOP_CONFIG_DIR "configs/development")

# 创建物理调试渲染器的选项，默认关闭
option(ENABLE_PHYSICS_DEBUG_RENDERER "Enable Physics Debug Renderer" OFF)

# 非Windows平台不支持物理调试渲染器
if(NOT WIN32)
  if(ENABLE_PHYSICS_DEBUG_RENDERER)
    # 显示警告信息并强制关闭该选项
    message(WARNING "Disable Physics Debug Renderer")
    set(ENABLE_PHYSICS_DEBUG_RENDERER OFF CACHE BOOL "" FORCE)
  endif()
endif()

# 如果启用物理调试渲染器，设置Jolt物理引擎的资源目录
if(ENABLE_PHYSICS_DEBUG_RENDERER)
  set(JOLT_ASSET_DIR "/jolt-asset")
endif()

# MSVC编译器特定设置
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    add_compile_options("/MP")
    set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT SammiEditor)
endif()

# 设置Vulkan头文件包含路径
set(vulkan_include ${THIRD_PARTY_DIR}/VulkanSDK/include)

# 平台特定的Vulkan设置
if(WIN32)
    # Windows平台设置Vulkan库文件
    set(vulkan_lib ${THIRD_PARTY_DIR}/VulkanSDK/lib/Win32/vulkan-1.lib)
    # Windows平台设置GLSL编译器路径
    set(glslangValidator_executable ${THIRD_PARTY_DIR}/VulkanSDK/bin/Win32/glslangValidator.exe)
    # 添加Vulkan层路径编译定义
    add_compile_definitions("PICCOLO_VK_LAYER_PATH=${THIRD_PARTY_DIR}/VulkanSDK/bin/Win32")
elseif(UNIX)
    if(APPLE)
      # macOS平台设置Vulkan库文件
      set(vulkan_lib ${THIRD_PARTY_DIR}/VulkanSDK/lib/MacOS/libvulkan.1.dylib)
      # macOS平台设置GLSL编译器路径
      set(glslangValidator_executable ${THIRD_PARTY_DIR}/VulkanSDK/bin/MacOS/glslangValidator)
      # 添加Vulkan层路径编译定义
      add_compile_definitions("PICCOLO_VK_LAYER_PATH=${THIRD_PARTY_DIR}/VulkanSDK/bin/MacOS")
      # 添加Vulkan安装客户端驱动(ICD)文件定义
      add_compile_definitions("PICCOLO_VK_ICD_FILENAMES=${THIRD_PARTY_DIR}/VulkanSDK/bin/MacOS/MoltenVK_icd.json")
    else()
      # Linux平台设置Vulkan库文件
      set(vulkan_lib ${THIRD_PARTY_DIR}/VulkanSDK/lib/Linux/libvulkan.so.1)
      # Linux平台设置GLSL编译器路径
      set(glslangValidator_executable ${THIRD_PARTY_DIR}/VulkanSDK/bin/Linux/glslangValidator)
      # 添加Vulkan层路径编译定义
      add_compile_definitions("PICCOLO_VK_LAYER_PATH=${THIRD_PARTY_DIR}/VulkanSDK/bin/Linux")
    endif()
else()
  # 未知平台报错退出
  message(FATAL_ERROR "Unknown Platform")
endif()

# 添加着色器编译目标并进入shader子目录
#set(SHADER_COMPILE_TARGET PiccoloShaderCompile)
#add_subdirectory(shader)

# 添加第三方库子目录
add_subdirectory(3rdparty)

# 添加运行时模块子目录
add_subdirectory(source/runtime)
# 添加编辑器模块子目录
add_subdirectory(source/editor)
# 添加元解析器模块子目录
add_subdirectory(source/meta_parser)
# 添加测试模块（当前已注释掉）
#add_subdirectory(source/test)

# 设置预编译目标名称并包含预编译配置
set(CODEGEN_TARGET "SammiPreCompile")
include(source/precompile/precompile.cmake)

# 在IDE中将目标归类到"Engine"文件夹
#set_target_properties("${CODEGEN_TARGET}" PROPERTIES FOLDER "Engine" )

# 设置预编译目标依赖关系
#add_dependencies(SammiRuntime "${CODEGEN_TARGET}")
#add_dependencies("${CODEGEN_TARGET}" "SammiParser")