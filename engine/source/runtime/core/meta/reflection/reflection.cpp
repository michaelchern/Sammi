#include "reflection.h"
#include <cstring>
#include <map>

namespace Sammi
{
    namespace Reflection
    {
        // 定义未知类型和未知名称的常量字符串
        const char* k_unknown_type = "UnknownType";
        const char* k_unknown      = "Unknown";

        // 全局反射注册表（存储所有注册的类型信息）
        static std::map<std::string, ClassFunctionTuple*>       m_class_map;  // 类名称 -> 类函数元组
        static std::multimap<std::string, FieldFunctionTuple*>  m_field_map;// 类名称 -> 字段函数元组（一对多）
        static std::multimap<std::string, MethodFunctionTuple*> m_method_map;// 类名称 -> 方法函数元组（一对多）
        static std::map<std::string, ArrayFunctionTuple*>       m_array_map;// 数组类型名称 -> 数组函数元组

        // 向字段映射注册字段信息
        void TypeMetaRegisterinterface::registerToFieldMap(const char* name, FieldFunctionTuple* value)
        {
            m_field_map.insert(std::make_pair(name, value));
        }

        // 向方法映射注册方法信息
        void TypeMetaRegisterinterface::registerToMethodMap(const char* name, MethodFunctionTuple* value)
        {
            m_method_map.insert(std::make_pair(name, value));
        }

        // 向数组映射注册数组信息
        void TypeMetaRegisterinterface::registerToArrayMap(const char* name, ArrayFunctionTuple* value)
        {
            // 确保同名数组类型只注册一次
            if (m_array_map.find(name) == m_array_map.end())
            {
                m_array_map.insert(std::make_pair(name, value));
            }
            else
            {
                delete value;  // 如果已存在则删除传入的值
            }
        }

        // 向类映射注册类信息
        void TypeMetaRegisterinterface::registerToClassMap(const char* name, ClassFunctionTuple* value)
        {
            // 确保同名类只注册一次
            if (m_class_map.find(name) == m_class_map.end())
            {
                m_class_map.insert(std::make_pair(name, value));
            }
            else
            {
                delete value;  // 如果已存在则删除传入的值
            }
        }

        // 清理所有注册的反射信息
        void TypeMetaRegisterinterface::unregisterAll()
        {
            // 清理字段映射
            for (const auto& itr : m_field_map)
            {
                delete itr.second;  // 释放内存
            }
            m_field_map.clear();

            // 清理类映射
            for (const auto& itr : m_class_map)
            {
                delete itr.second;
            }
            m_class_map.clear();

            // 清理数组映射
            for (const auto& itr : m_array_map)
            {
                delete itr.second;
            }
            m_array_map.clear();
        }

        // 类型元信息类的实现
        // =============================================

        // 通过类型名称构造TypeMeta
        TypeMeta::TypeMeta(std::string type_name) : m_type_name(type_name)
        {
            m_is_valid = false;  // 初始化为无效状态
            m_fields.clear();
            m_methods.clear();

            // 收集该类型的所有字段
            auto fileds_iter = m_field_map.equal_range(type_name);
            while (fileds_iter.first != fileds_iter.second)
            {
                FieldAccessor f_field(fileds_iter.first->second);
                m_fields.emplace_back(f_field);
                m_is_valid = true;  // 有字段存在则设为有效
                ++fileds_iter.first;
            }

            // 收集该类型的所有方法
            auto methods_iter = m_method_map.equal_range(type_name);
            while (methods_iter.first != methods_iter.second)
            {
                MethodAccessor f_method(methods_iter.first->second);
                m_methods.emplace_back(f_method);
                m_is_valid = true;  // 有方法存在则设为有效
                ++methods_iter.first;
            }
        }

        // 默认构造函数（未知类型）
        TypeMeta::TypeMeta() : m_type_name(k_unknown_type), m_is_valid(false)
        {
            // 初始化空列表
            m_fields.clear();
            m_methods.clear();
        }

        // 通过类型名获取TypeMeta对象
        TypeMeta TypeMeta::newMetaFromName(std::string type_name)
        {
            TypeMeta f_type(type_name);
            return f_type;
        }

        // 获取数组访问器
        bool TypeMeta::newArrayAccessorFromName(std::string array_type_name, ArrayAccessor& accessor)
        {
            auto iter = m_array_map.find(array_type_name);
            if (iter != m_array_map.end())
            {
                // 创建并返回数组访问器
                ArrayAccessor new_accessor(iter->second);
                accessor = new_accessor;
                return true;
            }

            return false;  // 未找到
        }

        // 从JSON创建对象实例
        ReflectionInstance TypeMeta::newFromNameAndJson(std::string type_name, const Json& json_context)
        {
            auto iter = m_class_map.find(type_name);
            if (iter != m_class_map.end())
            {
                // 使用类函数元组中的构造函数创建实例
                void* instance = std::get<1>(*iter->second)(json_context);
                // 返回反射实例（类型元信息+对象实例）
                return ReflectionInstance(TypeMeta(type_name), instance);
            }
            return ReflectionInstance();  // 返回无效实例
        }

        // 序列化对象到JSON
        Json TypeMeta::writeByName(std::string type_name, void* instance)
        {
            auto iter = m_class_map.find(type_name);
            if (iter != m_class_map.end())
            {
                // 使用类函数元组中的序列化函数
                return std::get<2>(*iter->second)(instance);
            }
            return Json();  // 返回空JSON
        }

        // 获取类型名称
        std::string TypeMeta::getTypeName() { return m_type_name; }

        // 获取字段访问器列表
        int TypeMeta::getFieldsList(FieldAccessor*& out_list)
        {
            int count = m_fields.size();
            out_list  = new FieldAccessor[count];
            for (int i = 0; i < count; ++i)
            {
                out_list[i] = m_fields[i];
            }
            return count;
        }

        // 获取方法访问器列表
        int TypeMeta::getMethodsList(MethodAccessor*& out_list)
        {
            int count = m_methods.size();
            out_list  = new MethodAccessor[count];
            for (int i = 0; i < count; ++i)
            {
                out_list[i] = m_methods[i];
            }
            return count;
        }

        // 获取基类反射实例列表（用于继承）
        int TypeMeta::getBaseClassReflectionInstanceList(ReflectionInstance*& out_list, void* instance)
        {
            auto iter = m_class_map.find(m_type_name);
            if (iter != m_class_map.end())
            {
                // 使用类函数元组中的基类获取函数
                return (std::get<0>(*iter->second))(out_list, instance);
            }
            return 0;
        }

        // 通过字段名称获取字段访问器
        FieldAccessor TypeMeta::getFieldByName(const char* name)
        {
            // 线性搜索匹配字段
            const auto it = std::find_if(m_fields.begin(), m_fields.end(), [&](const auto& i) {
                return std::strcmp(i.getFieldName(), name) == 0;
                });
            if (it != m_fields.end())
                return *it;
            return FieldAccessor(nullptr);  // 未找到
        }

        // 通过方法名称获取方法访问器
        MethodAccessor TypeMeta::getMethodByName(const char* name)
        {
            // 线性搜索匹配方法
            const auto it = std::find_if(m_methods.begin(), m_methods.end(), [&](const auto& i) {
                return std::strcmp(i.getMethodName(), name) == 0;
            });
            if (it != m_methods.end())
                return *it;
            return MethodAccessor(nullptr);
        }

        // 赋值操作符
        TypeMeta& TypeMeta::operator=(const TypeMeta& dest)
        {
            if (this == &dest)
            {
                return *this;
            }

            m_fields.clear();
            m_fields = dest.m_fields;

            m_methods.clear();
            m_methods = dest.m_methods;

            m_type_name = dest.m_type_name;
            m_is_valid  = dest.m_is_valid;

            return *this;
        }

        // 字段访问器类的实现
        // =============================================

        // 默认构造函数
        FieldAccessor::FieldAccessor()
        {
            m_field_type_name = k_unknown_type;
            m_field_name      = k_unknown;
            m_functions       = nullptr;
        }

        // 从字段函数元组构造
        FieldAccessor::FieldAccessor(FieldFunctionTuple* functions) : m_functions(functions)
        {
            m_field_type_name = k_unknown_type;
            m_field_name      = k_unknown;
            if (m_functions == nullptr)
            {
                return;
            }

            // 从元组中获取字段类型名称和字段名称
            m_field_type_name = (std::get<4>(*m_functions))();  // 字段类型名称
            m_field_name      = (std::get<3>(*m_functions))();  // 字段名称
        }

        // 获取字段值
        void* FieldAccessor::get(void* instance)
        {
            // 实际调用元组中的get函数
            return static_cast<void*>((std::get<1>(*m_functions))(instance));
        }

        // 设置字段值
        void FieldAccessor::set(void* instance, void* value)
        {
            // 实际调用元组中的set函数
            (std::get<0>(*m_functions))(instance, value);
        }

        // 获取拥有者类型元信息
        TypeMeta FieldAccessor::getOwnerTypeMeta()
        {
            // 从元组中获取拥有者类型名称
            TypeMeta f_type((std::get<2>(*m_functions))());
            return f_type;
        }

        // 获取字段类型元信息
        bool FieldAccessor::getTypeMeta(TypeMeta& field_type)
        {
            TypeMeta f_type(m_field_type_name);
            field_type = f_type;
            return f_type.m_is_valid;  // 返回类型是否有效
        }

        // 获取字段名称
        const char* FieldAccessor::getFieldName() const { return m_field_name; }

        // 获取字段类型名称
        const char* FieldAccessor::getFieldTypeName() { return m_field_type_name; }

        // 判断是否是数组类型
        bool FieldAccessor::isArrayType()
        {
            return (std::get<5>(*m_functions))();
        }

        // 赋值操作符
        FieldAccessor& FieldAccessor::operator=(const FieldAccessor& dest)
        {
            if (this == &dest)
            {
                return *this;
            }
            m_functions       = dest.m_functions;
            m_field_name      = dest.m_field_name;
            m_field_type_name = dest.m_field_type_name;
            return *this;
        }

        // 方法访问器类的实现
        // =============================================

        // 默认构造函数
        MethodAccessor::MethodAccessor()
        {
            m_method_name = k_unknown;
            m_functions   = nullptr;
        }

        // 从方法函数元组构造
        MethodAccessor::MethodAccessor(MethodFunctionTuple* functions) : m_functions(functions)
        {
            m_method_name      = k_unknown;
            if (m_functions == nullptr)
            {
                return;
            }

            // 从元组中获取方法名称
            m_method_name      = (std::get<0>(*m_functions))();
        }

        // 获取方法名称
        const char* MethodAccessor::getMethodName() const
        {
            return (std::get<0>(*m_functions))();
        }

        // 赋值操作符
        MethodAccessor& MethodAccessor::operator=(const MethodAccessor& dest)
        {
            if (this == &dest)
            {
                return *this;
            }
            m_functions       = dest.m_functions;
            m_method_name      = dest.m_method_name;
            return *this;
        }

        // 调用方法
        void MethodAccessor::invoke(void* instance)
        {
            // 实际调用元组中的调用函数
            (std::get<1>(*m_functions))(instance);
        }

        // 数组访问器类的实现
        // =============================================

        // 默认构造函数
        ArrayAccessor::ArrayAccessor() : m_func(nullptr), m_array_type_name("UnKnownType"), m_element_type_name("UnKnownType")
        {}

        // 从数组函数元组构造
        ArrayAccessor::ArrayAccessor(ArrayFunctionTuple* array_func) : m_func(array_func)
        {
            m_array_type_name   = k_unknown_type;
            m_element_type_name = k_unknown_type;
            if (m_func == nullptr)
            {
                return;
            }

            // 从元组中获取数组类型名称和元素类型名称
            m_array_type_name   = std::get<3>(*m_func)();
            m_element_type_name = std::get<4>(*m_func)();
        }

        // 获取数组类型名称
        const char* ArrayAccessor::getArrayTypeName() { return m_array_type_name; }

        // 获取元素类型名称
        const char* ArrayAccessor::getElementTypeName() { return m_element_type_name; }

        // 设置数组元素值
        void ArrayAccessor::set(int index, void* instance, void* element_value)
        {
            size_t count = getSize(instance);
            std::get<0> (*m_func)(index, instance, element_value);
        }

        // 获取数组元素值
        void* ArrayAccessor::get(int index, void* instance)
        {
            size_t count = getSize(instance);
            return std::get<1>(*m_func)(index, instance);
        }

        // 获取数组大小
        int ArrayAccessor::getSize(void* instance)
        {
            return std::get<2>(*m_func)(instance);
        }

        // 赋值操作符
        ArrayAccessor& ArrayAccessor::operator=(ArrayAccessor& dest)
        {
            if (this == &dest)
            {
                return *this;
            }
            m_func              = dest.m_func;
            m_array_type_name   = dest.m_array_type_name;
            m_element_type_name = dest.m_element_type_name;
            return *this;
        }

        // 反射实例类的实现
        // =============================================

        // 赋值操作符
        ReflectionInstance& ReflectionInstance::operator=(ReflectionInstance& dest)
        {
            if (this == &dest)
            {
                return *this;
            }
            m_instance = dest.m_instance;  // 复制实例指针
            m_meta     = dest.m_meta;      // 复制类型元信息
            return *this;
        }

        // 移动赋值操作符
        ReflectionInstance& ReflectionInstance::operator=(ReflectionInstance&& dest)
        {
            if (this == &dest)
            {
                return *this;
            }
            m_instance = dest.m_instance;  // 移动实例指针
            m_meta     = dest.m_meta;      // 移动类型元信息
            return *this;
        }
    }
}