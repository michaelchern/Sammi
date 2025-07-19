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
    option(GLFW_BUILD_EXAMPLES "" OFF)
    option(GLFW_BUILD_TESTS "" OFF)
    option(GLFW_BUILD_DOCS "" OFF)
    option(GLFW_INSTALL "" OFF)
    add_subdirectory(glfw)
    set_target_properties(glfw PROPERTIES FOLDER ${third_party_folder}/glfw)
    set_target_properties(update_mappings PROPERTIES FOLDER ${third_party_folder}/glfw)
endif()

if(NOT TARGET spdlog)
    option(SPDLOG_BUILD_EXAMPLE "" OFF)
    option(SPDLOG_INSTALL "" OFF)
    add_subdirectory(spdlog)
    set_target_properties(spdlog PROPERTIES FOLDER ${third_party_folder}/spdlog)
endif()

if(NOT TARGET stb)
    include(stb.cmake)
    set_target_properties(stb PROPERTIES FOLDER ${third_party_folder}/stb)
endif()

if(NOT TARGET tinyobjloader)
    add_subdirectory(tinyobjloader)
    set_target_properties(tinyobjloader PROPERTIES FOLDER ${third_party_folder}/tinyobjloader)
    set_target_properties(uninstall PROPERTIES FOLDER ${third_party_folder}/tinyobjloader)
endif()

if(NOT TARGET Jolt)
    option(TARGET_HELLO_WORLD "" OFF)
    option(TARGET_PERFORMANCE_TEST "" OFF)
    option(TARGET_SAMPLES "" OFF)
    option(TARGET_UNIT_TESTS "" OFF)
    option(TARGET_VIEWER "" OFF)

    if(ENABLE_PHYSICS_DEBUG_RENDERER)
        option(TARGET_TEST_FRAMEWORK "" ON)
    else()
        option(TARGET_TEST_FRAMEWORK "" OFF)
    endif()

    add_subdirectory(JoltPhysics/Build)

    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        include(CheckCXXCompilerFlag)
        check_cxx_compiler_flag("-Wno-unqualified-std-cast-call" COMPILER_CHECK_UNQUALIFIED)
        if(COMPILER_CHECK_UNQUALIFIED)
            target_compile_options(Jolt PRIVATE "-Wno-unqualified-std-cast-call")
        endif()
    endif()

    if(ENABLE_PHYSICS_DEBUG_RENDERER)
        set_target_properties(Jolt TestFramework
            PROPERTIES 
            FOLDER ${third_party_folder}/JoltPhysics
            MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
    else()
        set_target_properties(Jolt
            PROPERTIES 
            FOLDER ${third_party_folder}/JoltPhysics
            MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
    endif()

    if (MSVC)
        target_compile_options(Jolt PRIVATE "/W4")
    else()
        target_compile_options(Jolt PRIVATE "-Wall")
    endif()
endif()
