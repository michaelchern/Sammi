# ------------------------------- Ԥ���������ļ����� -------------------------------
# ����Ԥ���빤��·������ǰԴĿ¼�µ� bin �ļ��У�
set(PRECOMPILE_TOOLS_PATH "${CMAKE_CURRENT_SOURCE_DIR}/bin")
# ����Ԥ�������ģ���ļ�·�������룺δ����� precompile.json.in��
set(SAMMI_PRECOMPILE_PARAMS_IN_PATH "${CMAKE_CURRENT_SOURCE_DIR}/source/precompile/precompile.json.in")
# �������ɵ�Ԥ��������ļ�·��������������� precompile.json��
set(SAMMI_PRECOMPILE_PARAMS_PATH "${PRECOMPILE_TOOLS_PATH}/precompile.json")
# ��ģ���ļ����Ʋ������������յ�Ԥ��������ļ������滻���е� @����@ ռλ����
configure_file(${SAMMI_PRECOMPILE_PARAMS_IN_PATH} ${SAMMI_PRECOMPILE_PARAMS_PATH})

# ------------------------------- ƽ̨������ã�������·����ϵͳͷ�ļ��� -------------------------------
# ���ݵ�ǰ����ϵͳ��Windows/Linux/macOS������Ԥ������·����ϵͳ����·����sys_include��
if (CMAKE_HOST_WIN32)
    # Windows ϵͳ����
    set(PRECOMPILE_PRE_EXE)
    # Ԥ��������ִ���ļ���Windows �µ� .exe ��׺��
    set(PRECOMPILE_PARSER ${PRECOMPILE_TOOLS_PATH}/SammiParser.exe)
    # ϵͳͷ�ļ�����·����Windows �¿�����Ҫͨ������ض�·�����˴��� "*" ��ʾ����ϵͳͷ�ļ���
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

# ------------------------------- Ԥ���������ļ����� -------------------------------
# Ԥ������������ͷ�ļ�·�������ɵ�Ŀ��ͷ�ļ���д���·����
set (PARSER_INPUT ${CMAKE_BINARY_DIR}/parser_header.h)

# ------------------------------- ����Ŀ�꣺Ԥ�������� -------------------------------
### BUILDING ====================================================================================
# �����Զ��幹��Ŀ�꣨���ƣ�PiccoloPreCompile������ӵ� ALL ��ʾÿ�ι�������ִ��
set(PRECOMPILE_TARGET "SammiPreCompile")
add_custom_target(${PRECOMPILE_TARGET} ALL

# ������ע�ͣ������ô������ǿ��ÿ�α���ǰ�������� configure_file��ͨ������Ҫ��
# ${CMAKE_COMMAND} -E touch ${PRECOMPILE_PARAM_IN_PATH}a

# ���Ԥ���뿪ʼ��ʾ��Ϣ
COMMAND ${CMAKE_COMMAND} -E echo "************************************************************* "
COMMAND ${CMAKE_COMMAND} -E echo "**** [Precompile] BEGIN "
COMMAND ${CMAKE_COMMAND} -E echo "************************************************************* "

# ִ��Ԥ������������Ĳ�����
    # ����˵����
    # 1. Ԥ��������ļ�·�������ɵ� precompile.json��
    # 2. Ԥ��������ͷ�ļ�·����PARSER_INPUT��
    # 3. ��ĿԴ�����Ŀ¼��ENGINE_ROOT_DIR/source��
    # 4. ϵͳͷ�ļ�����·����sys_include��
    # 5. ��Ŀ���ƣ�"Sammi"��
    # 6. δ֪������0�����ܱ�ʾĳ��ģʽ��
COMMAND ${PRECOMPILE_PARSER} "${SAMMI_PRECOMPILE_PARAMS_PATH}"  "${PARSER_INPUT}"  "${ENGINE_ROOT_DIR}/source" ${sys_include} "Sammi" 0
# ���Ԥ���������ʾ��Ϣ
COMMAND ${CMAKE_COMMAND} -E echo "+++ Precompile finished +++"
