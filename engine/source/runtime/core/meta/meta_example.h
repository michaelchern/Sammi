#pragma once

#include "runtime/core/meta/reflection/reflection.h"  // 反射系统核心头文件

namespace Sammi
{
    // 定义基类 BaseTest 的反射信息
    REFLECTION_TYPE(BaseTest)            // 声明类型名称用于反射
    CLASS(BaseTest, Fields)              // 声明类反射特性（Fields表示反射所有字段）
    {
        REFLECTION_BODY(BaseTest);       // 开始类反射体定义

    public:
        int               m_int;         // 将被反射的整型字段
        std::vector<int*> m_int_vector;  // 将被反射的指针向量
    };

    // 定义派生类 Test1 的反射信息（使用白名单模式）
    REFLECTION_TYPE(Test1)
    CLASS(Test1 : public BaseTest, WhiteListFields)  // WhiteListFields表示只反射标记的字段
    {
        REFLECTION_BODY(Test1);                      // 开始类反射体定义

    public:
        META(Enable)                                 // 显式启用该字段的反射
        char m_char;                                 // 仅这个字段会被反射（基类字段不继承）
    };

    // 定义派生类 Test2 的反射信息（包含反射指针容器）
    REFLECTION_TYPE(Test2)
    CLASS(Test2 : public BaseTest, , Fields)  // 继承基类反射特性 + 启用全字段反射
    {
        REFLECTION_BODY(Test2);               // 开始类反射体定义

    public:
        // 存储反射基类指针的容器（支持多态）
        std::vector<Reflection::ReflectionPtr<BaseTest>> m_test_base_array;
    };
}