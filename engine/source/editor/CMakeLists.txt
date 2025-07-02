# 设置目标名称为SammiEditor
set(TARGET_NAME SammiEditor)

# 使用CONFIGURE_DEPENDS选项收集文件（当新文件添加时会自动重新配置）
# 收集编辑器头文件
file(GLOB EDITOR_HEADERS CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)
# 收集编辑器源文件
file(GLOB EDITOR_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp)
# 收集Windows资源文件（用于图标、版本信息等）
file(GLOB EDITOR_RESOURCE CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/resource/SammiEditor.rc)

# 在IDE中组织文件结构（按目录分组）
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${EDITOR_HEADERS} ${EDITOR_SOURCES} ${EDITOR_RESOURCE})

# 创建编辑器可执行文件目标
add_executable(${TARGET_NAME} ${EDITOR_HEADERS} ${EDITOR_SOURCES} ${EDITOR_RESOURCE})

# 定义预处理器宏：引擎根目录（运行时使用）
add_compile_definitions("SAMMI_ROOT_DIR=${BINARY_ROOT_DIR}")

# 设置包含目录：STB图像库路径
target_include_directories(
  ${TARGET_NAME} 
  PUBLIC $<BUILD_INTERFACE:${THIRD_PARTY_DIR}/stb>
)

# 设置目标属性
set_target_properties(${TARGET_NAME} PROPERTIES CXX_STANDARD 17 OUTPUT_NAME "SammiEditor")
set_target_properties(${TARGET_NAME} PROPERTIES FOLDER "Engine")

# 设置编译器选项：在MSVC上禁用警告视为错误
target_compile_options(${TARGET_NAME} PUBLIC "$<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/WX->")

# 链接依赖库：引擎运行时库
target_link_libraries(${TARGET_NAME} SammiRuntime)

# 构建后命令（在编译完成后执行的脚本）
set(POST_BUILD_COMMANDS
  # 创建二进制输出目录
  COMMAND ${CMAKE_COMMAND} -E make_directory "${BINARY_ROOT_DIR}"
  # 复制编辑器资源文件到输出目录
  COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/resource" "${BINARY_ROOT_DIR}/resource"
  # 复制生成的可执行文件及其依赖到输出目录
  COMMAND ${CMAKE_COMMAND} -E copy_directory "$<TARGET_FILE_DIR:${TARGET_NAME}>/" "${BINARY_ROOT_DIR}"
  # 复制部署配置文件
  COMMAND ${CMAKE_COMMAND} -E copy "${ENGINE_ROOT_DIR}/${DEPLOY_CONFIG_DIR}/${TARGET_NAME}.ini" "${BINARY_ROOT_DIR}"
  # 复制开发配置文件到可执行文件目录
  COMMAND ${CMAKE_COMMAND} -E copy "${ENGINE_ROOT_DIR}/${DEVELOP_CONFIG_DIR}/${TARGET_NAME}.ini" "$<TARGET_FILE_DIR:${TARGET_NAME}>/"
  # 清理并复制引擎资源文件
  COMMAND ${CMAKE_COMMAND} -E remove_directory "${BINARY_ROOT_DIR}/${ENGINE_ASSET_DIR}"
  COMMAND ${CMAKE_COMMAND} -E copy_directory "${ENGINE_ROOT_DIR}/${ENGINE_ASSET_DIR}" "${BINARY_ROOT_DIR}/${ENGINE_ASSET_DIR}"
)

# 如果启用物理调试渲染器，额外复制物理资源
if(ENABLE_PHYSICS_DEBUG_RENDERER)
  set(POST_BUILD_COMMANDS ${POST_BUILD_COMMANDS}
    COMMAND ${CMAKE_COMMAND} -E remove_directory "${BINARY_ROOT_DIR}/${JOLT_ASSET_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${ENGINE_ROOT_DIR}/${JOLT_ASSET_DIR}" "${BINARY_ROOT_DIR}/${JOLT_ASSET_DIR}"
)
endif()

# 将后处理命令添加到目标
add_custom_command(TARGET ${TARGET_NAME} ${POST_BUILD_COMMANDS})

# 预编译头设置：导出头文件列表到父作用域（中文引号需改为英文）
set(SAMMI_EDITOR_HEADS “${EDITOR_HEADERS}” PARENT_SCOPE)