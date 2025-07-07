# 设置目标名称变量为 SHADER_COMPILE_TARGET 的值
set(TARGET_NAME ${SHADER_COMPILE_TARGET})

# 递归查找所有 GLSL 着色器文件（包括多种扩展名）
file(GLOB_RECURSE SHADER_FILES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/glsl/*.vert"    # 顶点着色器
                                                 "${CMAKE_CURRENT_SOURCE_DIR}/glsl/*.frag"    # 片段着色器
                                                 "${CMAKE_CURRENT_SOURCE_DIR}/glsl/*.comp"    # 计算着色器
                                                 "${CMAKE_CURRENT_SOURCE_DIR}/glsl/*.geom"    # 几何着色器
                                                 "${CMAKE_CURRENT_SOURCE_DIR}/glsl/*.tesc"    # 曲面细分控制着色器
                                                 "${CMAKE_CURRENT_SOURCE_DIR}/glsl/*.tese"    # 曲面细分计算着色器
                                                 "${CMAKE_CURRENT_SOURCE_DIR}/glsl/*.mesh"    # 网格着色器
                                                 "${CMAKE_CURRENT_SOURCE_DIR}/glsl/*.task"    # 任务着色器
                                                 "${CMAKE_CURRENT_SOURCE_DIR}/glsl/*.rgen"    # 光线生成着色器
                                                 "${CMAKE_CURRENT_SOURCE_DIR}/glsl/*.rchit"   # 光线碰撞着色器
                                                 "${CMAKE_CURRENT_SOURCE_DIR}/glsl/*.rmiss"   # 光线未命中着色器
                                                 "${CMAKE_CURRENT_SOURCE_DIR}/glsl/*.rcall")  # 光线调用着色器

# 在 IDE 中创建 "shaders" 分组以组织文件
SOURCE_GROUP("shaders" FILES ${SHADER_FILES})

# 设置着色器头文件包含目录
set(SHADER_INCLUDE_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}/include)

# 定义生成文件的输出目录名称
set(GENERATED_SHADER_FOLDER "generated")
# 确保生成目录存在
if (NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${GENERATED_SHADER_FOLDER})
  file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${GENERATED_SHADER_FOLDER})
endif()

# 确保 SPIR-V 二进制文件的子目录存在
if (NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${GENERATED_SHADER_FOLDER}/spv)
  file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${GENERATED_SHADER_FOLDER}/spv)
endif()

# 引入自定义的着色器编译函数
include(${SAMMI_ROOT_DIR}/cmake/ShaderCompile.cmake)

# 调用着色器编译函数（核心操作）
compile_shader("${SHADER_FILES}"                  # 输入文件列表
               "${TARGET_NAME}"                   # 编译目标名称
               "${SHADER_INCLUDE_FOLDER}"         # 头文件包含路径
               "${GENERATED_SHADER_FOLDER}"       # 输出目录
               "${glslangValidator_executable}")  # glslangValidator 编译器路径

# 设置目标在 IDE 中的组织目录
set_target_properties("${TARGET_NAME}" PROPERTIES FOLDER "Engine" )

# For each shader, we create a header file
# foreach(SHADER ${shaders})
#     # Prepare a header name and a global variable for this shader
#     get_filename_component(SHADER_NAME ${SHADER} NAME)
#     string(REPLACE "." "_" HEADER_NAME ${SHADER_NAME})
#     string(TOUPPER ${HEADER_NAME} GLOBAL_SHADER_VAR)

#     set(SHADER_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/${GENERATED_SHADER_FOLDER}/${HEADER_NAME}.h")

#     add_custom_target(
#       ${HEADER_NAME}
#       # Compile any GLSL shader into SPIR-V shader
#       # -g -Od should only be used in debug version
#       COMMAND ${glslangValidator_executable} -I${SHADER_INCLUDE_FOLDER} -V100 -o ${CMAKE_CURRENT_SOURCE_DIR}/${GENERATED_SHADER_FOLDER}/${SHADER_NAME}.spv ${SHADER}
#       # Make a C header file with the SPIR-V shader
#       COMMAND ${CMAKE_COMMAND} -DPATH="${CMAKE_CURRENT_SOURCE_DIR}/${GENERATED_SHADER_FOLDER}/${SHADER_NAME}.spv" -DHEADER="${SHADER_HEADER}" -DGLOBAL="${GLOBAL_SHADER_VAR}" -P "${CMAKE_CURRENT_SOURCE_DIR}/embed_resource.cmake"
#       # Rebuild the header file if the shader is updated
#       DEPENDS ${SHADER}
#       COMMENT "Building ${CMAKE_CURRENT_SOURCE_DIR}/${GENERATED_SHADER_FOLDER}/${SHADER_NAME}.spv and embedding it into ${SHADER_HEADER}"
#     )

#     set_target_properties(${HEADER_NAME} PROPERTIES FOLDER "Shaders/generated" )
#     # Add the custom target like a dependencies of the project
#     add_dependencies(${PICCOLO_NAME} ${HEADER_NAME})

#     message(STATUS "Generating build commands for ${SHADER}")
# endforeach()

# 以下是被注释掉的替代实现方案
# 说明：这部分代码展示了另一种实现方式，但当前未被使用
# --------------------------------------------------------------
# # 为每个着色器创建单独的头文件
# foreach(SHADER ${shaders})
#     # 获取基本文件名并格式化为C标识符
#     get_filename_component(SHADER_NAME ${SHADER} NAME)
#     string(REPLACE "." "_" HEADER_NAME ${SHADER_NAME})
#     string(TOUPPER ${HEADER_NAME} GLOBAL_SHADER_VAR)
#
#     # 设置生成的头文件路径
#     set(SHADER_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/${GENERATED_SHADER_FOLDER}/${HEADER_NAME}.h")
#
#     # 为每个着色器创建自定义编译命令
#     add_custom_target(
#         ${HEADER_NAME}
#         # 步骤1: 编译GLSL为SPIR-V
#         COMMAND ${glslangValidator_executable} -I${SHADER_INCLUDE_FOLDER} -V100 
#                -o ${CMAKE_CURRENT_SOURCE_DIR}/${GENERATED_SHADER_FOLDER}/${SHADER_NAME}.spv 
#                ${SHADER}
#         # 步骤2: 将SPIR-V嵌入C头文件
#         COMMAND ${CMAKE_COMMAND} 
#                -DPATH="${CMAKE_CURRENT_SOURCE_DIR}/${GENERATED_SHADER_FOLDER}/${SHADER_NAME}.spv" 
#                -DHEADER="${SHADER_HEADER}" 
#                -DGLOBAL="${GLOBAL_SHADER_VAR}" 
#                -P "${CMAKE_CURRENT_SOURCE_DIR}/embed_resource.cmake"
#         # 依赖关系声明
#         DEPENDS ${SHADER}
#         COMMENT "Building and embedding shader: ${SHADER_NAME}"
#     )
#
#     # 设置IDE中的分组
#     set_target_properties(${HEADER_NAME} PROPERTIES FOLDER "Shaders/generated")
#     # 添加对主项目的依赖
#     add_dependencies(${PICCOLO_NAME} ${HEADER_NAME})
#
#     message(STATUS "Generating build commands for ${SHADER}")
# endforeach()
# --------------------------------------------------------------