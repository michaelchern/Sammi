#pragma once

#include "runtime/core/meta/json.h"  // JSON序列化支持

#include <functional>     // 函数对象
#include <string>         // 字符串处理
#include <unordered_map>  // 哈希映射
#include <unordered_set>  // 哈希集合
#include <vector>         // 动态数组

namespace Sammi
{

// 反射解析器预处理指令
// 当__REFLECTION_PARSER__定义时，启用反射注解
#if defined(__REFLECTION_PARSER__)
#define META(...) __attribute__((annotate(#__VA_ARGS__)))
#define CLASS(class_name, ...) class __attribute__((annotate(#__VA_ARGS__))) class_name
#define STRUCT(struct_name, ...) struct __attribute__((annotate(#__VA_ARGS__))) struct_name
//#define CLASS(class_name,...) class __attribute__((annotate(#__VA_ARGS__))) class_name:public Reflection::object
#else
// 非解析器环境下，注解宏置空
#define META(...)
#define CLASS(class_name, ...) class class_name
#define STRUCT(struct_name, ...) struct struct_name
//#define CLASS(class_name,...) class class_name:public Reflection::object
#endif // __REFLECTION_PARSER__

// 反射主体宏（放在类定义中）
#define REFLECTION_BODY(class_name) \
    friend class Reflection::TypeFieldReflectionOparator::Type##class_name##Operator; \  // 声明反射操作器为友元
    friend class Serializer;  // 声明序列化器为友元
    // public: virtual std::string getTypeName() override {return #class_name;}

// 反射类型声明宏（放在类外部）
#define REFLECTION_TYPE(class_name) \
    namespace Reflection \
    { \
        namespace TypeFieldReflectionOparator \
        { \
            class Type##class_name##Operator; \  // 声明反射操作器类
        } \
    };

// 注册宏定义
#define REGISTER_FIELD_TO_MAP(name, value) TypeMetaRegisterinterface::registerToFieldMap(name, value);       // 注册字段
#define REGISTER_Method_TO_MAP(name, value) TypeMetaRegisterinterface::registerToMethodMap(name, value);     // 注册方法
#define REGISTER_BASE_CLASS_TO_MAP(name, value) TypeMetaRegisterinterface::registerToClassMap(name, value);  // 注册基类
#define REGISTER_ARRAY_TO_MAP(name, value) TypeMetaRegisterinterface::registerToArrayMap(name, value);       // 注册数组
#define UNREGISTER_ALL TypeMetaRegisterinterface::unregisterAll();                                           // 取消所有注册

// 反射对象创建/删除宏
#define SAMMI_REFLECTION_NEW(name, ...) Reflection::ReflectionPtr(#name, new name(__VA_ARGS__));  // 反射方式创建对象
#define SAMMI_REFLECTION_DELETE(value) \
    if (value) \
    { \
        delete value.operator->(); \  // 删除指针指向的对象
        value.getPtrReference() = nullptr; \  // 置空指针
    }
#define SAMMI_REFLECTION_DEEP_COPY(type, dst_ptr, src_ptr) \
    *static_cast<type*>(dst_ptr) = *static_cast<type*>(src_ptr.getPtr());  // 深拷贝对象

// 类型元定义宏
#define TypeMetaDef(class_name, ptr) \
    Piccolo::Reflection::ReflectionInstance(Piccolo::Reflection::TypeMeta::newMetaFromName(#class_name), \
                                            (class_name*)ptr)  // 创建类型元实例

#define TypeMetaDefPtr(class_name, ptr) \
    new Piccolo::Reflection::ReflectionInstance(Piccolo::Reflection::TypeMeta::newMetaFromName(#class_name), \
                                                (class_name*)ptr)  // 创建类型元指针


    // 安全类型转换检查
    template<typename T, typename U, typename = void>
    struct is_safely_castable : std::false_type {};  // 默认不能安全转换

    template<typename T, typename U>
    struct is_safely_castable<T, U, std::void_t<decltype(static_cast<U>(std::declval<T>()))>> : std::true_type {};  // 可转换时特化为true

    namespace Reflection
    {
        // 前向声明
        class TypeMeta;
        class FieldAccessor;
        class MethodAccessor;
        class ArrayAccessor;
        class ReflectionInstance;
    }

    // 函数类型定义
    typedef std::function<void(void*, void*)>      SetFuncion;      // 字段设置函数
    typedef std::function<void*(void*)>            GetFuncion;      // 字段获取函数
    typedef std::function<const char*()>           GetNameFuncion;  // 名称获取函数
    typedef std::function<void(int, void*, void*)> SetArrayFunc;    // 数组元素设置函数
    typedef std::function<void*(int, void*)>       GetArrayFunc;    // 数组元素获取函数
    typedef std::function<int(void*)>              GetSizeFunc;     // 数组大小获取函数
    typedef std::function<bool()>                  GetBoolFunc;     // 布尔值获取函数
    typedef std::function<void(void*)>             InvokeFunction;  // 方法调用函数

    // 类相关函数类型定义
    typedef std::function<void*(const Json&)>                           ConstructorWithJson;                     // JSON构造函数
    typedef std::function<Json(void*)>                                  WriteJsonByName;                         // JSON序列化函数
    typedef std::function<int(Reflection::ReflectionInstance*&, void*)> GetBaseClassReflectionInstanceListFunc;  // 基类反射实例获取函数

    // 函数元组定义
    typedef std::tuple<SetFuncion, GetFuncion, GetNameFuncion, GetNameFuncion, GetNameFuncion, GetBoolFunc> FieldFunctionTuple;   // 字段函数元组
    typedef std::tuple<GetNameFuncion, InvokeFunction>                                                      MethodFunctionTuple;  // 方法函数元组
    typedef std::tuple<GetBaseClassReflectionInstanceListFunc, ConstructorWithJson, WriteJsonByName>        ClassFunctionTuple;   // 类函数元组
    typedef std::tuple<SetArrayFunc, GetArrayFunc, GetSizeFunc, GetNameFuncion, GetNameFuncion>             ArrayFunctionTuple;   // 数组函数元组

    namespace Reflection
    {
        // 类型元注册接口
        class TypeMetaRegisterinterface
        {
        public:
            // 注册函数声明（实现在其它文件中）
            static void registerToClassMap(const char* name, ClassFunctionTuple* value);
            static void registerToFieldMap(const char* name, FieldFunctionTuple* value);
            static void registerToMethodMap(const char* name, MethodFunctionTuple* value);
            static void registerToArrayMap(const char* name, ArrayFunctionTuple* value);
            static void unregisterAll();  // 取消所有注册
        };

        // 类型元信息类（核心反射数据结构）
        class TypeMeta
        {
            friend class FieldAccessor;              // 字段访问器友元
            friend class ArrayAccessor;              // 数组访问器友元
            friend class TypeMetaRegisterinterface;  // 注册接口友元

        public:
            TypeMeta();  // 默认构造函数
            // static void Register();
            static TypeMeta           newMetaFromName(std::string type_name);                                             // 通过类型名创建类型元
            static bool               newArrayAccessorFromName(std::string array_type_name, ArrayAccessor& accessor);     // 获取数组访问器
            static ReflectionInstance newFromNameAndJson(std::string type_name, const Json& json_context);                // JSON反序列化创建实例
            static Json               writeByName(std::string type_name, void* instance);                                 // 序列化对象到JSON
            std::string               getTypeName();                                                                      // 获取类型名称
            int                       getFieldsList(FieldAccessor*& out_list);                                            // 获取字段访问器列表
            int                       getMethodsList(MethodAccessor*& out_list);                                          // 获取方法访问器列表
            int                       getBaseClassReflectionInstanceList(ReflectionInstance*& out_list, void* instance);  // 获取基类反射实例列表
            FieldAccessor             getFieldByName(const char* name);                                                   // 通过字段名获取字段访问器
            MethodAccessor            getMethodByName(const char* name);                                                  // 通过方法名获取方法访问器
            bool                      isValid() { return m_is_valid; }                                                    // 类型元是否有效
            TypeMeta& operator=(const TypeMeta& dest);                                                                    // 赋值运算符

        private:
            TypeMeta(std::string type_name);                                          // 带类型名的构造函数

        private:
            std::vector<FieldAccessor, std::allocator<FieldAccessor>>   m_fields;     // 字段访问器列表
            std::vector<MethodAccessor, std::allocator<MethodAccessor>> m_methods;    // 方法访问器列表
            std::string                                                 m_type_name;  // 类型名称
            bool                                                        m_is_valid;   // 是否有效标志
        };

        // 字段访问器类
        class FieldAccessor
        {
            friend class TypeMeta;                                // 类型元友元

        public:
            FieldAccessor();                                      // 默认构造函数
            void*       get(void* instance);                      // 获取字段值
            void        set(void* instance, void* value);         // 设置字段值
            TypeMeta    getOwnerTypeMeta();                       // 获取所属类型元
            bool        getTypeMeta(TypeMeta& field_type);        // 获取字段类型元
            const char* getFieldName() const;                     // 获取字段名
            const char* getFieldTypeName();                       // 获取字段类型名
            bool        isArrayType();                            // 是否为数组类型
            FieldAccessor& operator=(const FieldAccessor& dest);  // 赋值运算符

        private:
            FieldAccessor(FieldFunctionTuple* functions);         // 通过函数元组构造

        private:
            FieldFunctionTuple* m_functions;                      // 字段函数元组指针
            const char*         m_field_name;                     // 字段名称
            const char*         m_field_type_name;                // 字段类型名
        };

        // 方法访问器类
        class MethodAccessor
        {
            friend class TypeMeta;                                  // 类型元友元

        public:
            MethodAccessor();                                       // 默认构造函数
            void invoke(void* instance);                            // 调用方法
            const char* getMethodName() const;                      // 获取方法名
            MethodAccessor& operator=(const MethodAccessor& dest);  // 赋值运算符

        private:
            MethodAccessor(MethodFunctionTuple* functions);         // 通过函数元组构造

        private:
            MethodFunctionTuple* m_functions;                       // 方法函数元组指针
            const char*          m_method_name;                     // 方法名称
        };

        // 数组访问器类（当前实现为标准向量访问器）
        class ArrayAccessor
        {
            friend class TypeMeta;                                            // 类型元友元

        public:
            ArrayAccessor();                                                  // 默认构造函数
            const char* getArrayTypeName();                                   // 获取数组类型名
            const char* getElementTypeName();                                 // 获取元素类型名
            void        set(int index, void* instance, void* element_value);  // 设置数组元素
            void*       get(int index, void* instance);                       // 获取数组元素
            int         getSize(void* instance);                              // 获取数组大小
            ArrayAccessor& operator=(ArrayAccessor& dest);                    // 赋值运算符

        private:
            ArrayAccessor(ArrayFunctionTuple* array_func);                    // 通过函数元组构造

        private:
            ArrayFunctionTuple* m_func;                                       // 数组函数元组指针
            const char*         m_array_type_name;                            // 数组类型名
            const char*         m_element_type_name;                          // 元素类型名
        };

        // 反射实例类（类型元+实例指针）
        class ReflectionInstance
        {
        public:
            ReflectionInstance(TypeMeta meta, void* instance) : m_meta(meta), m_instance(instance) {}  // 构造函数
            ReflectionInstance() : m_meta(), m_instance(nullptr) {}                                    // 默认构造函数
            ReflectionInstance& operator=(ReflectionInstance& dest);                                   // 赋值运算符
            ReflectionInstance& operator=(ReflectionInstance&& dest);                                  // 移动赋值运算符

        public:
            TypeMeta m_meta;      // 类型元信息
            void*    m_instance;  // 实例指针
        };

        // 反射指针模板类（安全指针包装）
        template<typename T>
        class ReflectionPtr
        {
            template<typename U>
            friend class ReflectionPtr;  // 模板友元

        public:
            // 构造函数
            ReflectionPtr(std::string type_name, T* instance) : m_type_name(type_name), m_instance(instance) {}
            ReflectionPtr() : m_type_name(), m_instance(nullptr) {}
            ReflectionPtr(const ReflectionPtr& dest) : m_type_name(dest.m_type_name), m_instance(dest.m_instance) {}

            // 赋值运算符（类型转换）
            template<typename U /*, typename = typename std::enable_if<std::is_safely_castable<T*, U*>::value>::type */>
            ReflectionPtr<T>& operator=(const ReflectionPtr<U>& dest)
            {
                if (this == static_cast<void*>(&dest))
                {
                    return *this;
                }
                m_type_name = dest.m_type_name;
                m_instance  = static_cast<T*>(dest.m_instance);
                return *this;
            }

            // 移动赋值运算符（类型转换）
            template<typename U /*, typename = typename std::enable_if<std::is_safely_castable<T*, U*>::value>::type*/>
            ReflectionPtr<T>& operator=(ReflectionPtr<U>&& dest)
            {
                if (this == static_cast<void*>(&dest))
                {
                    return *this;
                }
                m_type_name = dest.m_type_name;
                m_instance  = static_cast<T*>(dest.m_instance);
                return *this;
            }

            // 赋值运算符（同类型）
            ReflectionPtr<T>& operator=(const ReflectionPtr<T>& dest)
            {
                if (this == &dest)
                {
                    return *this;
                }
                m_type_name = dest.m_type_name;
                m_instance  = dest.m_instance;
                return *this;
            }

            // 移动赋值运算符（同类型）
            ReflectionPtr<T>& operator=(ReflectionPtr<T>&& dest)
            {
                if (this == &dest)
                {
                    return *this;
                }
                m_type_name = dest.m_type_name;
                m_instance  = dest.m_instance;
                return *this;
            }

            // 类型信息
            std::string getTypeName() const { return m_type_name; }
            void setTypeName(std::string name) { m_type_name = name; }

            // 比较运算符
            bool operator==(const T* ptr) const { return (m_instance == ptr); }
            bool operator!=(const T* ptr) const { return (m_instance != ptr); }
            bool operator==(const ReflectionPtr<T>& rhs_ptr) const { return (m_instance == rhs_ptr.m_instance); }
            bool operator!=(const ReflectionPtr<T>& rhs_ptr) const { return (m_instance != rhs_ptr.m_instance); }

            // 类型转换运算符
            template<typename T1 /*, typename = typename std::enable_if<std::is_safely_castable<T*, T1*>::value>::type*/>
            explicit operator T1*()
            {
                return static_cast<T1*>(m_instance);
            }
            template<typename T1 /*, typename = typename std::enable_if<std::is_safely_castable<T*, T1*>::value>::type*/>
            operator ReflectionPtr<T1>()
            {
                return ReflectionPtr<T1>(m_type_name, (T1*)(m_instance));
            }
            template<typename T1 /*, typename = typename std::enable_if<std::is_safely_castable<T*, T1*>::value>::type*/>
            explicit operator const T1*() const
            {
                return static_cast<T1*>(m_instance);
            }
            template<typename T1 /*, typename = typename std::enable_if<std::is_safely_castable<T*, T1*>::value>::type*/>
            operator const ReflectionPtr<T1>() const
            {
                return ReflectionPtr<T1>(m_type_name, (T1*)(m_instance));
            }

            // 成员访问运算符
            T* operator->() { return m_instance; }
            T* operator->() const { return m_instance; }
            T& operator*() { return *(m_instance); }
            T* getPtr() { return m_instance; }
            T* getPtr() const { return m_instance; }
            const T& operator*() const { return *(static_cast<const T*>(m_instance)); }
            T*& getPtrReference() { return m_instance; }

            // 布尔转换（指针有效性检查）
            operator bool() const { return (m_instance != nullptr); }

        private:
            std::string m_type_name {""};      // 类型名称
            typedef T   m_type;                // 类型别名
            T*          m_instance {nullptr};  // 对象指针
        };
    }
}