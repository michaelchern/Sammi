// 这段代码是编辑器的主程序入口。它启动引擎和编辑器，并运行编辑器的主循环。

#include <filesystem>               // 文件系统库，用于路径操作
#include <iostream>                 // 输入输出流
#include <string>                   // 字符串处理
#include <thread>                   // 多线程支持
#include <unordered_map>            // 哈希表容器

#include "runtime/engine.h"         // 引擎核心头文件
#include "editor/include/editor.h"  // 编辑器模块头文件

// 宏定义：用于字符串化预处理变量
// https://gcc.gnu.org/onlinedocs/cpp/Stringizing.html
#define SAMMI_XSTR(s) SAMMI_STR(s)  // 双重展开宏
#define SAMMI_STR(s) #s             // 字符串化操作符

int main(int argc, char** argv)
{
    // 获取可执行文件路径
    std::filesystem::path executable_path(argv[0]);
    // 构造配置文件路径（与可执行文件同目录）
    std::filesystem::path config_file_path = executable_path.parent_path() / "SammiEditor.ini";

    // 创建引擎实例
    Sammi::SammiEngine* engine = new Sammi::SammiEngine();

    // 启动引擎并初始化（传入配置文件路径）
    engine->startEngine(config_file_path.generic_string());
    engine->initialize();

    // 创建编辑器实例
    Sammi::SammiEditor* editor = new Sammi::SammiEditor();
    // 使用引擎初始化编辑器
    editor->initialize(engine);

    // 运行编辑器主循环
    editor->run();

    // 清理编辑器资源
    editor->clear();

    // 清理引擎资源并关闭
    engine->clear();
    engine->shutdownEngine();

    return 0;  // 程序正常退出
}