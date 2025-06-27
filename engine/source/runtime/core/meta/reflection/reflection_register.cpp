
#include <assert.h>                                            // 断言支持，用于调试

// 核心头文件
#include "runtime/core/meta/json.h"                            // JSON支持
#include "runtime/core/meta/reflection/reflection.h"           // 反射核心
#include "runtime/core/meta/reflection/reflection_register.h"  // 反射注册接口
#include "runtime/core/meta/serializer/serializer.h"           // 序列化支持

// 自动生成的反射和序列化代码
#include "_generated/reflection/all_reflection.h"              // 包含所有反射注册
#include "_generated/serializer/all_serializer.ipp"            // 包含所有序列化实现

namespace Sammi
{
    namespace Reflection
    {
        // 反射系统反注册函数实现
        void TypeMetaRegister::metaUnregister()
        {
            // 调用底层接口清理所有注册的反射数据
            TypeMetaRegisterinterface::unregisterAll();
        }
    }
}