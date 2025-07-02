# 设置预编译工具路径和配置文件路径
set(PRECOMPILE_TOOLS_PATH "${CMAKE_CURRENT_SOURCE_DIR}/bin")  # 预编译工具所在目录
set(SAMMI_PRECOMPILE_PARAMS_IN_PATH "${CMAKE_CURRENT_SOURCE_DIR}/source/precompile/precompile.json.in")  # 输入配置文件模板
set(SAMMI_PRECOMPILE_PARAMS_PATH "${PRECOMPILE_TOOLS_PATH}/precompile.json")  # 最终配置文件路径

# 将模板文件转换为实际配置文件（处理变量替换）
configure_file(
    ${SAMMI_PRECOMPILE_PARAMS_IN_PATH}  # 输入文件路径
    ${SAMMI_PRECOMPILE_PARAMS_PATH})    # 输出文件路径

# 平台特定的预编译器配置
if (CMAKE_HOST_WIN32)
    # Windows 配置
    set(PRECOMPILE_PRE_EXE)  # 不需要特殊前缀
    set(PRECOMPILE_PARSER ${PRECOMPILE_TOOLS_PATH}/SammiParser.exe)  # 可执行文件路径
    set(sys_include "*")  # Windows 包含路径通配符
elseif(${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Linux" )
    # Linux 配置
    set(PRECOMPILE_PRE_EXE)
    set(PRECOMPILE_PARSER ${PRECOMPILE_TOOLS_PATH}/SammiParser)  # 可执行文件路径
    set(sys_include "/usr/include/c++/9/")  # 指定标准库路径
    # 可选：设置可执行权限（已注释掉）
    #execute_process(COMMAND chmod a+x ${PRECOMPILE_PARSER} WORKING_DIRECTORY ${PRECOMPILE_TOOLS_PATH})
elseif(CMAKE_HOST_APPLE)
    # macOS 配置
    # 查找 xcrun 工具（用于获取 macOS SDK 信息）
    find_program(XCRUN_EXECUTABLE xcrun)
    if(NOT XCRUN_EXECUTABLE)
      message(FATAL_ERROR "xcrun not found!!!")  # 未找到则报错
    endif()
    # 获取 macOS SDK 平台路径
    execute_process(
      COMMAND ${XCRUN_EXECUTABLE} --sdk macosx --show-sdk-platform-path
      OUTPUT_VARIABLE osx_sdk_platform_path_test
      OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    set(PRECOMPILE_PRE_EXE)  # 不需要特殊前缀
    set(PRECOMPILE_PARSER ${PRECOMPILE_TOOLS_PATH}/SammiParser)  # 可执行文件路径
    # 设置标准库包含路径（使用Xcode工具链）
    set(sys_include "${osx_sdk_platform_path_test}/../../Toolchains/XcodeDefault.xctoolchain/usr/include/c++/v1") 
endif()

# 设置解析器的输出文件路径（在构建目录中）
set (PARSER_INPUT ${CMAKE_BINARY_DIR}/parser_header.h)

### 构建预编译目标 ===================================================================
set(PRECOMPILE_TARGET "SammiPreCompile")  # 目标名称

# 创建自定义目标（ALL表示包含在默认构建中）
add_custom_target(${PRECOMPILE_TARGET} ALL

# 调试命令：被注释掉，不推荐使用（会导致每次构建都重新生成配置文件）
# ${CMAKE_COMMAND} -E touch ${PRECOMPILE_PARAM_IN_PATH}a
    
# 打印开始信息
COMMAND
  ${CMAKE_COMMAND} -E echo "************************************************************* "
COMMAND
  ${CMAKE_COMMAND} -E echo "**** [Precompile] BEGIN "
COMMAND
  ${CMAKE_COMMAND} -E echo "************************************************************* "

# 运行预编译解析器（关键步骤）
COMMAND
    ${PRECOMPILE_PARSER}
    "${PICCOLO_PRECOMPILE_PARAMS_PATH}"  # 参数1：配置文件路径
    "${PARSER_INPUT}"                    # 参数2：输出头文件路径
    "${ENGINE_ROOT_DIR}/source"          # 参数3：引擎源代码目录
    ${sys_include}                       # 参数4：系统包含路径
    "Sammi"                              # 参数5：项目名称
    0                                    # 参数6：附加标志（例如0可能表示安静模式）

# 打印完成信息
COMMAND ${CMAKE_COMMAND} -E echo "+++ Precompile finished +++"
)