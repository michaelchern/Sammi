#pragma once

namespace Sammi
{
    namespace Reflection
    {
        // 类型元注册器类
        // 提供全局反射系统的注册和注销接口
        class TypeMetaRegister
        {
        public:
            // 执行反射系统的全局注册
            // 调用此函数将注册所有反射类型（类、字段、方法等）
            // 通常在程序初始化时调用一次
            static void metaRegister();

            // 执行反射系统的全局注销
            // 清理所有已注册的反射数据，释放资源
            // 通常在程序退出时调用
            static void metaUnregister();
        };
    }
}