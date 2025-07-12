# ============================== 第三方库基础配置 ==============================
# 定义第三方库的统一存放目录名称（用于 IDE 中分组显示）
set(third_party_folder "ThirdParty")

# 强制所有第三方库以静态库形式构建（避免动态库链接问题，简化部署）
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

# ============================== ImGui（跨平台 GUI 库） ==============================
#if(NOT TARGET imgui)
#    include(imgui.cmake)
#    set_target_properties(imgui PROPERTIES FOLDER ${third_party_folder}/imgui)
#endif()

