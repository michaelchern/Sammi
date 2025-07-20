# ---- 定义编辑器目标名称 ----
set(TARGET_NAME SammiEditor)

# ---- 收集源文件与资源文件 ----
# 递归扫描当前目录下的 include 目录，收集所有 .h 头文件（CONFIGURE_DEPENDS 表示文件变化时触发 CMake 重新配置）
file(GLOB EDITOR_HEADERS CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)
# 递归扫描当前目录下的 source 目录，收集所有 .cpp 源文件
file(GLOB EDITOR_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp)
# 收集 Windows 资源文件（.rc 文件，用于图标、版本信息等）
file(GLOB EDITOR_RESOURCE CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/resource/PiccoloEditor.rc)

# ---- IDE 源文件分组（可选，提升 IDE 可读性） ----
# 根据源文件的实际目录结构，在 IDE 中显示为层级化的文件夹（如 include/ 下的文件归到 "include" 组）
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${EDITOR_HEADERS} ${EDITOR_SOURCES} ${EDITOR_RESOURCE})

# ---- 创建可执行文件目标 ----
# 将头文件、源文件、资源文件编译为可执行文件（Windows 下为 .exe，Linux/macOS 下为无后缀可执行文件）
add_executable(${TARGET_NAME} ${EDITOR_HEADERS} ${EDITOR_SOURCES} ${EDITOR_RESOURCE})

# ---- 预处理器定义（传递路径信息到代码） ----
# 定义 SAMMI_ROOT_DIR 为二进制根目录（代码中可用此宏定位资源或配置文件路径）
add_compile_definitions("SAMMI_ROOT_DIR=${BINARY_ROOT_DIR}")

# ---- 包含目录配置 ----
# 设置 PiccoloEditor 的公共包含目录（PUBLIC 表示依赖此目标的其他目标也会继承此路径）
# 这里引入 stb 图像/数学库的头文件路径（THIRD_PARTY_DIR 是之前定义的第三方库根目录）
target_include_directories(
  ${TARGET_NAME} 
  PUBLIC $<BUILD_INTERFACE:${THIRD_PARTY_DIR}/stb>
)

# ---- 编译选项与属性设置 ----
# 设置 C++ 标准为 17（与项目整体标准一致）
set_target_properties(${TARGET_NAME} PROPERTIES CXX_STANDARD 17)
# 设置输出文件名（避免默认带版本号或随机后缀，直接命名为 SammiEditor）
set_target_properties(${TARGET_NAME} PROPERTIES OUTPUT_NAME "PiccoloEditor")
# 在 IDE 中将目标分组到 "Engine" 文件夹（便于管理）
set_target_properties(${TARGET_NAME} PROPERTIES FOLDER "Engine")

# ---- 编译器特定选项（MSVC）----
# 仅对 MSVC 编译器生效：将警告视为错误（/WX 选项），强制修复潜在问题
target_compile_options(${TARGET_NAME} PUBLIC "$<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/WX->")

# ---- 链接依赖库 ----
# 将 SammiEditor 链接到 SammiRuntime 库（运行时核心功能，如渲染、物理等）
target_link_libraries(${TARGET_NAME} SammiRuntime)

# ---- 构建后操作（复制资源与配置文件）----
# 定义构建后需要执行的命令列表（按顺序执行）
set(POST_BUILD_COMMANDS
  # 1. 创建二进制根目录（若不存在）
  COMMAND ${CMAKE_COMMAND} -E make_directory "${BINARY_ROOT_DIR}"
  # 2. 复制当前编辑器的资源目录（如图标、UI 资源）到输出目录
  COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/resource" "${BINARY_ROOT_DIR}/resource"
  # 3. 复制可执行文件自身到输出目录（可能用于动态加载或调试）
  COMMAND ${CMAKE_COMMAND} -E copy_directory "$<TARGET_FILE_DIR:${TARGET_NAME}>/" "${BINARY_ROOT_DIR}"
  # 4. 复制部署环境配置文件（如发布版参数）到输出目录
  COMMAND ${CMAKE_COMMAND} -E copy "${ENGINE_ROOT_DIR}/${DEPLOY_CONFIG_DIR}/${TARGET_NAME}.ini" "${BINARY_ROOT_DIR}"
  # 5. 复制开发环境配置文件到可执行文件所在目录（调试时使用）
  COMMAND ${CMAKE_COMMAND} -E copy "${ENGINE_ROOT_DIR}/${DEVELOP_CONFIG_DIR}/${TARGET_NAME}.ini" "$<TARGET_FILE_DIR:${TARGET_NAME}>/"
  # 6. 删除默认的资源占位目录（避免与实际资源冲突）
  COMMAND ${CMAKE_COMMAND} -E remove_directory "${BINARY_ROOT_DIR}/${ENGINE_ASSET_DIR}"
  # 7. 复制引擎内置资源（如模型、纹理）到输出目录
  COMMAND ${CMAKE_COMMAND} -E copy_directory "${ENGINE_ROOT_DIR}/${ENGINE_ASSET_DIR}" "${BINARY_ROOT_DIR}/${ENGINE_ASSET_DIR}"
)

# ---- 物理调试渲染器额外资源复制（条件执行）----
# 若启用了物理调试渲染器（ENABLE_PHYSICS_DEBUG_RENDERER），复制 Jolt 物理引擎的调试资源
if(ENABLE_PHYSICS_DEBUG_RENDERER)
  set(POST_BUILD_COMMANDS ${POST_BUILD_COMMANDS}
    # 删除默认的 Jolt 资源占位目录
    COMMAND ${CMAKE_COMMAND} -E remove_directory "${BINARY_ROOT_DIR}/${JOLT_ASSET_DIR}"
    # 复制 Jolt 物理引擎的调试资源（如碰撞形状可视化数据）到输出目录
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${ENGINE_ROOT_DIR}/${JOLT_ASSET_DIR}" "${BINARY_ROOT_DIR}/${JOLT_ASSET_DIR}"
)
endif()

# ---- 绑定构建后命令到目标 ----
# 当 SammiEditor 构建完成后，执行 POST_BUILD_COMMANDS 中的所有命令
add_custom_command(TARGET ${TARGET_NAME} ${POST_BUILD_COMMANDS})

# ---- 预编译相关（传递头文件列表到父作用域）----
# 设置 SAMMI_EDITOR_HEADS 变量（值为当前编辑器的头文件列表），供父 CMakeLists.txt 使用
# 可能用于预编译步骤（如生成代码、反射信息或其他预处理任务）
set(SAMMI_EDITOR_HEADS “${EDITOR_HEADERS}” PARENT_SCOPE)
