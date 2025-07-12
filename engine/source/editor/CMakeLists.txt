# ============================== 目标定义与基础配置 ==============================
# 定义当前模块的目标名称（编辑器可执行文件）
set(TARGET_NAME SammiEditor)

# 收集当前目录下的头文件、源文件和资源文件（CONFIGURE_DEPENDS 表示 CMake 重新配置时会重新扫描文件）
file(GLOB EDITOR_HEADERS CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)
file(GLOB EDITOR_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp)
file(GLOB EDITOR_RESOURCE CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/resource/SammiEditor.rc) # Windows 资源文件（如版本信息、图标等）

# 在 IDE（如 Visual Studio）中按目录结构分组文件，提升可读性
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${EDITOR_HEADERS} ${EDITOR_SOURCES} ${EDITOR_RESOURCE})

# 创建可执行文件目标（编辑器程序）
add_executable(${TARGET_NAME} ${EDITOR_HEADERS} ${EDITOR_SOURCES} ${EDITOR_RESOURCE})

# ============================== 编译配置 ==============================
# 添加预编译定义：指定引擎根目录（构建时路径，可能用于资源加载或配置文件路径）
add_compile_definitions("SAMMI_ROOT_DIR=${BINARY_ROOT_DIR}")

# 设置头文件包含路径（PUBLIC 表示依赖此目标的其他目标也会继承该路径）
target_include_directories(
  ${TARGET_NAME} 
  PUBLIC $<BUILD_INTERFACE:${THIRD_PARTY_DIR}/stb>
)

# 设置 C++ 标准为 C++17
set_target_properties(${TARGET_NAME} PROPERTIES CXX_STANDARD 17)
# 设置输出文件名（避免编译器自动生成带版本号的后缀）
set_target_properties(${TARGET_NAME} PROPERTIES OUTPUT_NAME "SammiEditor")
# 在 IDE 中将目标归类到 "Engine" 文件夹（提升项目结构清晰度）
set_target_properties(${TARGET_NAME} PROPERTIES FOLDER "Engine")

# 针对 MSVC 编译器添加严格警告选项（/WX 将警告视为错误）
target_compile_options(${TARGET_NAME} PUBLIC "$<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/WX->")

# ============================== 链接与依赖 ==============================
# 链接运行时库（编辑器依赖核心运行时功能）
target_link_libraries(${TARGET_NAME} SammiRuntime)

# ============================== 构建后操作（POST_BUILD） ==============================
# 定义构建完成后需要执行的命令列表（用于复制资源、配置文件等）
set(POST_BUILD_COMMANDS
  # 1. 创建二进制根目录（如果不存在）
  COMMAND ${CMAKE_COMMAND} -E make_directory "${BINARY_ROOT_DIR}"

  # 2. 复制资源目录（如纹理、模型、UI 等）
  COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/resource" "${BINARY_ROOT_DIR}/resource"

  # 3. 复制当前构建生成的编辑器可执行文件到二进制根目录（可能用于快速启动或测试）
  COMMAND ${CMAKE_COMMAND} -E copy_directory "$<TARGET_FILE_DIR:${TARGET_NAME}>/" "${BINARY_ROOT_DIR}"

  # 4. 复制部署环境配置文件（如服务器地址、发布模式参数）
  COMMAND ${CMAKE_COMMAND} -E copy "${ENGINE_ROOT_DIR}/${DEPLOY_CONFIG_DIR}/${TARGET_NAME}.ini" "${BINARY_ROOT_DIR}"

  # 5. 复制开发环境配置文件到可执行文件同级目录（开发时调试参数）
  COMMAND ${CMAKE_COMMAND} -E copy "${ENGINE_ROOT_DIR}/${DEVELOP_CONFIG_DIR}/${TARGET_NAME}.ini" "$<TARGET_FILE_DIR:${TARGET_NAME}>/"

  # 6. 清理旧版引擎资产目录（避免残留旧资源）
  COMMAND ${CMAKE_COMMAND} -E remove_directory "${BINARY_ROOT_DIR}/${ENGINE_ASSET_DIR}"

  # 7. 复制最新引擎资产到输出目录（如模型、贴图、脚本等）
  COMMAND ${CMAKE_COMMAND} -E copy_directory "${ENGINE_ROOT_DIR}/${ENGINE_ASSET_DIR}" "${BINARY_ROOT_DIR}/${ENGINE_ASSET_DIR}"
)

# 如果启用了物理调试渲染器，额外复制 Jolt 物理引擎的调试资源
if(ENABLE_PHYSICS_DEBUG_RENDERER)
  set(POST_BUILD_COMMANDS ${POST_BUILD_COMMANDS}
    # 清理旧版 Jolt 调试资产目录
    COMMAND ${CMAKE_COMMAND} -E remove_directory "${BINARY_ROOT_DIR}/${JOLT_ASSET_DIR}"
    # 复制最新 Jolt 调试资源到输出目录
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${ENGINE_ROOT_DIR}/${JOLT_ASSET_DIR}" "${BINARY_ROOT_DIR}/${JOLT_ASSET_DIR}"
)
endif()

# 将构建后命令绑定到 SammiEditor 目标的 POST_BUILD 阶段（每次构建完成后自动执行）
add_custom_command(TARGET ${TARGET_NAME} POST_BUILD ${POST_BUILD_COMMANDS})

# ============================== 预编译相关（注释部分） ==============================
# （以下为注释掉的预编译配置，可能用于生成代码或反射）
# # 设置预编译阶段需要的全局变量（头文件列表）
set(SAMMI_EDITOR_HEADS “${EDITOR_HEADERS}” PARENT_SCOPE)
