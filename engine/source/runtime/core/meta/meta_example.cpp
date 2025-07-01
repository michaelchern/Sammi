#include "runtime/core/meta/meta_example.h"        // 包含反射示例头文件
#include "_generated/serializer/all_serializer.h"  // 自动生成的序列化器

#include "runtime/core/base/macro.h"  // 核心宏定义
#include <filesystem>
#include <fstream>
#include <iostream>

namespace Sammi
{
    void metaExample()
    {
        //========== Test1 对象的序列化/反序列化演示 ==========//
        Test1 test1_in;                          // 创建测试对象
        test1_in.m_int  = 12;                    // 设置基本类型字段
        test1_in.m_char = 'g';                   // 设置字符字段
        int i           = 1;
        test1_in.m_int_vector.emplace_back(&i);  // 向vector添加指针

        Test1 test1_out;                         // 用于接收反序列化的对象
        
        //========== Test2 对象的构造 ==========//
        Test2 test2_in;
        // 向基类指针vector添加元素（支持多态）
        test2_in.m_test_base_array.emplace_back("Test1", &test1_in);  // 命名构造：显式指定类型
        Test1 Test2_temp;
        test2_in.m_test_base_array.emplace_back("Test1", &Test2_temp);  // 添加第二个元素

        //================= 序列化操作 =================//
        // 将test1_in对象序列化为JSON
        auto test1_json_in = Serializer::write(test1_in);
        std::string test1_context = test1_json_in.dump();  // 转为字符串

        // 反序列化：从JSON字符串重建对象
        std::string err;  // 存储可能的错误信息
        auto&& Test1_json = Json::parse(test1_context, err);  // 解析JSON
        Serializer::read(Test1_json, test1_out);              // 填充test1_out对象
        LOG_INFO(test1_context);                              // 打印序列化结果

        //================= Test2 序列化到文件 ================//
        auto        Test2_json_in = Serializer::write(test2_in);  // 序列化Test2
        std::string test2_context = Test2_json_in.dump();
        // 写入文件（可用于持久化/网络传输）
        std::fstream out_put("out.txt", std::ios::out);
        out_put << test2_context;
        out_put.flush();
        out_put.close();

        //=============== Test2 从文件反序列化 ===============//
        Test2  test2_out;
        auto&& test2_json = Json::parse(test2_context, err);
        Serializer::read(test2_json, test2_out);  // 重建test2_out对象
        LOG_INFO(test2_context.c_str());          // 打印结果

        //================= 反射运行时操作 =================//
        // 获取test2_out对象的类型元信息
        auto meta = TypeMetaDef(Test2, &test2_out);

        // 获取所有字段反射信息
        Reflection::FieldAccessor* fields;
        int fields_count = meta.m_meta.getFieldsList(fields);

        // 遍历字段反射信息
        for (int i = 0; i < fields_count; ++i)
        {
            auto filed_accesser = fields[i];
            // 打印字段基本信息
            std::cout << filed_accesser.getFieldTypeName() << " " << filed_accesser.getFieldName() << " "
                      << (char*)filed_accesser.get(meta.m_instance) << std::endl;
            if (filed_accesser.isArrayType())
            {
                Reflection::ArrayAccessor array_accesser;
                if (Reflection::TypeMeta::newArrayAccessorFromName(filed_accesser.getFieldTypeName(), array_accesser))
                {
                    void* field_instance = filed_accesser.get(meta.m_instance);
                    int   count          = array_accesser.getSize(field_instance);
                    auto  typeMetaItem   = Reflection::TypeMeta::newMetaFromName(array_accesser.getElementTypeName());
                    for (int index = 0; index < count; ++index)
                    {
                        std::cout << ":L:" << index << ":R:" << (int*)array_accesser.get(index, field_instance)
                                  << std::endl;
                        ;
                    }
                }
            }
        }
    }
}