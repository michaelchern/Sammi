// 包含标准库头文件
#include <filesystem>  // 用于文件系统操作（路径处理）
#include <iostream>    // 用于输入输出流操作
#include <string>      // 用于字符串处理
#include <thread>        // 用于多线程支持（当前代码未直接使用）
#include <unordered_map>   // 用于哈希表容器（当前代码未直接使用）

// 包含项目自定义头文件
#include "runtime/engine.h"  // 引擎核心功能头文件（定义SammiEngine类）
#include "editor/include/editor.h"  // 编辑器功能头文件（定义SammiEditor类）

// GCC编译器支持的字符串化宏技巧（将宏参数转换为字符串字面量）
// SAMMI_XSTR(s) 先调用SAMMI_STR(s)，用于处理带参数的宏场景（当前代码未直接使用）
// SAMMI_STR(s) 将宏参数s转换为字符串字面量（例如SAMMI_STR(abc)会变成"abc"）
#define SAMMI_XSTR(s) SAMMI_STR(s)
#define SAMMI_STR(s) #s

int main(int argc, char** argv)
{
    // 获取当前可执行文件的路径（argv[0]存储程序自身的路径）
    std::filesystem::path executable_path(argv[0]);

    // 构造配置文件路径：可执行文件所在目录 + "SammiEditor.ini"
    // parent_path()获取可执行文件的父目录（即程序所在文件夹）
    // operator/ 用于拼接路径（跨平台兼容Windows和Linux的路径分隔符）
    std::filesystem::path config_file_path = executable_path.parent_path() / "SammiEditor.ini";

    // 创建引擎实例（动态内存分配，需后续手动释放或通过其他机制管理）
    Sammi::SammiEngine* engine = new Sammi::SammiEngine();

    // 启动引擎并加载配置文件
    // generic_string()将std::filesystem::path转换为平台无关的字符串格式（如Windows的"\\", Linux的"/"）
    engine->startEngine(config_file_path.generic_string());

    // 初始化引擎（完成内部资源加载、模块初始化等操作）
    engine->initialize();

    // 创建编辑器实例（动态内存分配）
    Sammi::SammiEditor* editor = new Sammi::SammiEditor();

    // 初始化编辑器，并将引擎实例关联到编辑器（建立两者通信关系）
    editor->initialize(engine);

    // 运行编辑器主循环（阻塞执行，直到编辑器关闭）
    editor->run();

    // 清理编辑器资源（释放编辑器内部占用的内存、关闭子窗口等）
    editor->clear();

    // 清理引擎资源（释放引擎内部资源，如渲染上下文、物理引擎等）
    engine->clear();

    // 关闭引擎（执行引擎级别的清理操作，如保存全局配置、断开外部连接等）
    engine->shutdownEngine();

    // 注意：当前代码未显式释放engine和editor的内存（new分配的内存未delete）
    // 可能项目中通过clear()/shutdownEngine()内部实现了内存释放，或使用智能指针管理
    // 实际开发中需注意内存泄漏问题（建议改用std::unique_ptr等RAII机制）

    return 0;  // 程序正常退出
}
