#pragma once

#include "precompiled.h"
// 包含预编译头文件，通常用于加速编译过程

// 前置声明（避免包含完整头文件）
class Class;     // 表示一个类/结构体
class Global;    // 表示全局变量/函数
class Function;  // 表示独立函数
class Enum;      // 表示枚举类型

// 描述代码模块结构的元数据结构体
struct SchemaModule
{
    std::string name;  // 模块名称（如命名空间/包名）

    // 存储模块内所有类的智能指针数组
    // 使用shared_ptr实现自动内存管理
    std::vector<std::shared_ptr<Class>> classes;

    // 注：Global/Function/Enum未使用，可能是为后续扩展预留
};