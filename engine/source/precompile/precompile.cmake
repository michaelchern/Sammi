# ------------------------------- 预编译配置文件生成 -------------------------------
# 设置预编译工具路径（当前源目录下的 bin 文件夹）
set(PRECOMPILE_TOOLS_PATH "${CMAKE_CURRENT_SOURCE_DIR}/bin")
# 设置预编译参数模板文件路径（输入：未处理的 precompile.json.in）
set(SAMMI_PRECOMPILE_PARAMS_IN_PATH "${CMAKE_CURRENT_SOURCE_DIR}/source/precompile/precompile.json.in")
# 设置生成的预编译参数文件路径（输出：处理后的 precompile.json）
set(SAMMI_PRECOMPILE_PARAMS_PATH "${PRECOMPILE_TOOLS_PATH}/precompile.json")
# 将模板文件复制并处理生成最终的预编译参数文件（会替换其中的 @变量@ 占位符）
configure_file(${SAMMI_PRECOMPILE_PARAMS_IN_PATH} ${SAMMI_PRECOMPILE_PARAMS_PATH})

# ------------------------------- 平台相关配置（解析器路径、系统头文件） -------------------------------
# 根据当前主机系统（Windows/Linux/macOS）设置预编译器路径和系统包含路径（sys_include）
if (CMAKE_HOST_WIN32)
    # Windows 系统配置
    set(PRECOMPILE_PRE_EXE)
    # 预编译器可执行文件（Windows 下的 .exe 后缀）
    set(PRECOMPILE_PARSER ${PRECOMPILE_TOOLS_PATH}/SammiParser.exe)
    # 系统头文件包含路径（Windows 下可能需要通配符或特定路径，此处用 "*" 表示所有系统头文件）
    set(sys_include "*") 
elseif(${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Linux" )
    set(PRECOMPILE_PRE_EXE)
    set(PRECOMPILE_PARSER ${PRECOMPILE_TOOLS_PATH}/SammiParser)
    set(sys_include "/usr/include/c++/9/") 
    #execute_process(COMMAND chmod a+x ${PRECOMPILE_PARSER} WORKING_DIRECTORY ${PRECOMPILE_TOOLS_PATH})
elseif(CMAKE_HOST_APPLE)
    find_program(XCRUN_EXECUTABLE xcrun)
    if(NOT XCRUN_EXECUTABLE)
      message(FATAL_ERROR "xcrun not found!!!")
    endif()

    execute_process(
      COMMAND ${XCRUN_EXECUTABLE} --sdk macosx --show-sdk-platform-path
      OUTPUT_VARIABLE osx_sdk_platform_path_test
      OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    set(PRECOMPILE_PRE_EXE)
    set(PRECOMPILE_PARSER ${PRECOMPILE_TOOLS_PATH}/SammiParser)
    set(sys_include "${osx_sdk_platform_path_test}/../../Toolchains/XcodeDefault.xctoolchain/usr/include/c++/v1") 
endif()

# ------------------------------- 预编译输入文件定义 -------------------------------
# 预编译器的输入头文件路径（生成的目标头文件会写入此路径）
set (PARSER_INPUT ${CMAKE_BINARY_DIR}/parser_header.h)

# ------------------------------- 构建目标：预编译任务 -------------------------------
### BUILDING ====================================================================================
# 定义自定义构建目标（名称：PiccoloPreCompile），添加到 ALL 表示每次构建都会执行
set(PRECOMPILE_TARGET "SammiPreCompile")
add_custom_target(${PRECOMPILE_TARGET} ALL

# （调试注释）若启用此命令，会强制每次编译前重新生成 configure_file（通常不需要）
# ${CMAKE_COMMAND} -E touch ${PRECOMPILE_PARAM_IN_PATH}a

# 输出预编译开始提示信息
COMMAND ${CMAKE_COMMAND} -E echo "************************************************************* "
COMMAND ${CMAKE_COMMAND} -E echo "**** [Precompile] BEGIN "
COMMAND ${CMAKE_COMMAND} -E echo "************************************************************* "

# 执行预编译器命令（核心操作）
    # 参数说明：
    # 1. 预编译参数文件路径（生成的 precompile.json）
    # 2. 预编译输入头文件路径（PARSER_INPUT）
    # 3. 项目源代码根目录（ENGINE_ROOT_DIR/source）
    # 4. 系统头文件包含路径（sys_include）
    # 5. 项目名称（"Sammi"）
    # 6. 未知参数（0，可能表示某种模式）
COMMAND ${PRECOMPILE_PARSER} "${SAMMI_PRECOMPILE_PARAMS_PATH}"  "${PARSER_INPUT}"  "${ENGINE_ROOT_DIR}/source" ${sys_include} "Sammi" 0
# 输出预编译完成提示信息
COMMAND ${CMAKE_COMMAND} -E echo "+++ Precompile finished +++"
