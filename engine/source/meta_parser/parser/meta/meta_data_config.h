#pragma once

// 元数据属性常量命名空间
// 包含所有用于控制代码生成的注解标记名称
namespace NativeProperty
{
    // 全局控制标记（用于类/字段/方法）
    const auto All = "All";  // 全部内容标记（最高优先级）

    // 分类标记（作用域控制）
    const auto Fields = "Fields";    // 字段相关标记（控制字段生成）
    const auto Methods = "Methods";  // 方法相关标记（控制方法生成）

    // 启用/禁用标记（细粒度控制）
    const auto Enable  = "Enable";   // 显式启用标记（白名单模式）
    const auto Disable = "Disable";  // 显式禁用标记（黑名单模式）

    // 白名单控制标记（类级别控制策略）
    const auto WhiteListFields = "WhiteListFields";    // 字段白名单模式
    const auto WhiteListMethods = "WhiteListMethods";  // 方法白名单模式

}