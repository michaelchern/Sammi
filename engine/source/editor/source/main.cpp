// ������׼��ͷ�ļ�
#include <filesystem>  // �����ļ�ϵͳ������·������
#include <iostream>    // �����������������
#include <string>      // �����ַ�������
#include <thread>        // ���ڶ��߳�֧�֣���ǰ����δֱ��ʹ�ã�
#include <unordered_map>   // ���ڹ�ϣ����������ǰ����δֱ��ʹ�ã�

// ������Ŀ�Զ���ͷ�ļ�
#include "runtime/engine.h"  // ������Ĺ���ͷ�ļ�������SammiEngine�ࣩ
#include "editor/include/editor.h"  // �༭������ͷ�ļ�������SammiEditor�ࣩ

// GCC������֧�ֵ��ַ������꼼�ɣ��������ת��Ϊ�ַ�����������
// SAMMI_XSTR(s) �ȵ���SAMMI_STR(s)�����ڴ���������ĺ곡������ǰ����δֱ��ʹ�ã�
// SAMMI_STR(s) �������sת��Ϊ�ַ���������������SAMMI_STR(abc)����"abc"��
#define SAMMI_XSTR(s) SAMMI_STR(s)
#define SAMMI_STR(s) #s

int main(int argc, char** argv)
{
    // ��ȡ��ǰ��ִ���ļ���·����argv[0]�洢���������·����
    std::filesystem::path executable_path(argv[0]);

    // ���������ļ�·������ִ���ļ�����Ŀ¼ + "SammiEditor.ini"
    // parent_path()��ȡ��ִ���ļ��ĸ�Ŀ¼�������������ļ��У�
    // operator/ ����ƴ��·������ƽ̨����Windows��Linux��·���ָ�����
    std::filesystem::path config_file_path = executable_path.parent_path() / "SammiEditor.ini";

    // ��������ʵ������̬�ڴ���䣬������ֶ��ͷŻ�ͨ���������ƹ���
    Sammi::SammiEngine* engine = new Sammi::SammiEngine();

    // �������沢���������ļ�
    // generic_string()��std::filesystem::pathת��Ϊƽ̨�޹ص��ַ�����ʽ����Windows��"\\", Linux��"/"��
    engine->startEngine(config_file_path.generic_string());

    // ��ʼ�����棨����ڲ���Դ���ء�ģ���ʼ���Ȳ�����
    engine->initialize();

    // �����༭��ʵ������̬�ڴ���䣩
    Sammi::SammiEditor* editor = new Sammi::SammiEditor();

    // ��ʼ���༭������������ʵ���������༭������������ͨ�Ź�ϵ��
    editor->initialize(engine);

    // ���б༭����ѭ��������ִ�У�ֱ���༭���رգ�
    editor->run();

    // ����༭����Դ���ͷű༭���ڲ�ռ�õ��ڴ桢�ر��Ӵ��ڵȣ�
    editor->clear();

    // ����������Դ���ͷ������ڲ���Դ������Ⱦ�����ġ���������ȣ�
    engine->clear();

    // �ر����棨ִ�����漶�������������籣��ȫ�����á��Ͽ��ⲿ���ӵȣ�
    engine->shutdownEngine();

    // ע�⣺��ǰ����δ��ʽ�ͷ�engine��editor���ڴ棨new������ڴ�δdelete��
    // ������Ŀ��ͨ��clear()/shutdownEngine()�ڲ�ʵ�����ڴ��ͷţ���ʹ������ָ�����
    // ʵ�ʿ�������ע���ڴ�й©���⣨�������std::unique_ptr��RAII���ƣ�

    return 0;  // ���������˳�
}
