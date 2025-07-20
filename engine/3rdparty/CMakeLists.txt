# ---- 第三方库统一管理配置 ----
# 定义第三方库根目录名称（用于 IDE 中分组显示）
set(third_party_folder "ThirdParty")

# 强制关闭第三方库的共享库构建（所有第三方库均编译为静态库）
# CACHE BOOL "" FORCE 表示覆盖用户原有配置并强制生效
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

# ---- ImGui 图形 UI 库构建 ----
# 若项目中尚未生成 ImGui 目标（避免重复构建）
if(NOT TARGET imgui)
    # 包含 ImGui 的 CMake 构建脚本（通常由 ImGui 官方或项目自定义提供）
    include(imgui.cmake)
    # 设置 ImGui 目标在 IDE 中的文件夹分组（便于管理）
    set_target_properties(imgui PROPERTIES FOLDER ${third_party_folder}/imgui)
endif()

# ---- GLFW 窗口/输入库构建 ----
if(NOT TARGET glfw)
    # 关闭 GLFW 的示例、测试、文档构建（减少编译时间和冗余输出）
    option(GLFW_BUILD_EXAMPLES "" OFF)  # 不构建示例程序
    option(GLFW_BUILD_TESTS "" OFF)     # 不构建测试用例
    option(GLFW_BUILD_DOCS "" OFF)      # 不生成文档
    option(GLFW_INSTALL "" OFF)         # 不执行安装步骤（项目自行管理安装路径）
    # 添加 GLFW 源码子目录（触发其 CMake 构建流程）
    add_subdirectory(glfw)
    # 设置 GLFW 主目标在 IDE 中的文件夹分组
    set_target_properties(glfw PROPERTIES FOLDER ${third_party_folder}/glfw)
    # 设置 GLFW 生成的映射文件目标（如输入事件映射）的分组
    set_target_properties(update_mappings PROPERTIES FOLDER ${third_party_folder}/glfw)
endif()

# ---- spdlog 日志库构建 ----
if(NOT TARGET spdlog)
    # 关闭 spdlog 的示例构建和安装（项目自行管理头文件包含）
    option(SPDLOG_BUILD_EXAMPLE "" OFF)
    option(SPDLOG_INSTALL "" OFF)
    add_subdirectory(spdlog)
    set_target_properties(spdlog PROPERTIES FOLDER ${third_party_folder}/spdlog)
endif()

# ---- stb 图像/数学库构建 ----
if(NOT TARGET stb)
    # 包含 stb 的 CMake 构建脚本（stb 通常通过单个头文件实现，此处可能为自定义封装）
    include(stb.cmake)
    set_target_properties(stb PROPERTIES FOLDER ${third_party_folder}/stb)
endif()

# ---- tinyobjloader 模型加载库构建 ----
if(NOT TARGET tinyobjloader)
    add_subdirectory(tinyobjloader)
    set_target_properties(tinyobjloader PROPERTIES FOLDER ${third_party_folder}/tinyobjloader)
    set_target_properties(uninstall PROPERTIES FOLDER ${third_party_folder}/tinyobjloader)
endif()

#if(NOT TARGET Jolt)
#    option(TARGET_HELLO_WORLD "" OFF)
#    option(TARGET_PERFORMANCE_TEST "" OFF)
#    option(TARGET_SAMPLES "" OFF)
#    option(TARGET_UNIT_TESTS "" OFF)
#    option(TARGET_VIEWER "" OFF)

#    if(ENABLE_PHYSICS_DEBUG_RENDERER)
#        option(TARGET_TEST_FRAMEWORK "" ON)
#    else()
#        option(TARGET_TEST_FRAMEWORK "" OFF)
#    endif()

#    add_subdirectory(JoltPhysics/Build)

#    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
#        include(CheckCXXCompilerFlag)
#        check_cxx_compiler_flag("-Wno-unqualified-std-cast-call" COMPILER_CHECK_UNQUALIFIED)
#        if(COMPILER_CHECK_UNQUALIFIED)
#            target_compile_options(Jolt PRIVATE "-Wno-unqualified-std-cast-call")
#        endif()
#    endif()

#    if(ENABLE_PHYSICS_DEBUG_RENDERER)
#        set_target_properties(Jolt TestFramework
#            PROPERTIES 
#            FOLDER ${third_party_folder}/JoltPhysics
#            MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
#    else()
#        set_target_properties(Jolt
#            PROPERTIES 
#            FOLDER ${third_party_folder}/JoltPhysics
#            MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
#    endif()

#    if (MSVC)
#        target_compile_options(Jolt PRIVATE "/W4")
#    else()
#        target_compile_options(Jolt PRIVATE "-Wall")
#    endif()
#endif()
