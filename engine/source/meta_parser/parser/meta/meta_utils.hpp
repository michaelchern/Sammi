namespace Utils
{
    // 通用范围比较函数模板
    // 比较两个序列[startA, endA)和[startB, endB)是否完全相等
    // 类型要求：
    //   A, B: 前向迭代器类型
    //   元素类型需支持operator!=
    template<typename A, typename B>
    bool rangeEqual(A startA, A endA, B startB, B endB)
    {
        // 并行遍历两个序列
        while (startA != endA && startB != endB)
        {
            // 比较当前元素
            if (*startA != *startB)
                return false;  // 元素不相等时立即返回

            // 移动到下一元素
            ++startA;
            ++startB;
        }

        // 检查是否同时到达序列末尾
        return (startA == endA) && (startB == endB);
    }
}