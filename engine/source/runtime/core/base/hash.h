#pragma once

#include <cstddef>
#include <functional>  // 包含std::hash

// 基础版本：组合单个值的哈希到种子
// seed: 当前累积的哈希值（引用传递，会被修改）
// v: 需要合并哈希的目标对象
template<typename T>
inline void hash_combine(std::size_t& seed, const T& v)
{
    // 经典哈希组合算法（参考Boost实现）：
    // 1. 计算对象v的标准哈希值
    // 2. 加入黄金比例魔数(0x9e3779b9)增加随机性
    // 3. 通过位移操作(seed<<6 / seed>>2)混合原种子值
    // 4. 使用异或操作组合所有部分
    seed ^= std::hash<T> {}(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

// 可变参数版本：组合多个值的哈希到种子
// seed: 当前累积的哈希值（引用传递）
// v: 当前要处理的第一个对象
// rest: 剩余要处理的对象包
template<typename T, typename... Ts>
inline void hash_combine(std::size_t& seed, const T& v, Ts... rest)
{
    // 递归处理参数包中的每个值
    // 步骤1: 先合并第一个参数v
    hash_combine(seed, v);

    // 步骤2: 递归处理剩余参数（编译时判断）
    // 使用if constexpr确保递归在编译时展开
    if constexpr (sizeof...(Ts) > 1)  // 错误：实际应为 > 0，此条件会跳过单个参数
    {
        // 继续递归处理参数包中的剩余参数
        hash_combine(seed, rest...);
    }
    // 潜在问题：当rest只有一个参数时，sizeof...(Ts)==1，
    // 条件不成立导致最后一个参数未被处理
}