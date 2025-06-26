#pragma once

// 基本数值操作宏
////////////////////////////////////////////////////////////////////////////////

// 求两个数的最小值（支持整型和浮点型）
// 注意：宏参数会被多次展开，确保传入简单表达式
#define SAMMI_MIN(x, y) (((x) < (y)) ? (x) : (y))

// 求两个数的最大值（支持整型和浮点型）
// 注意：参数会被多次展开
#define SAMMI_MAX(x, y) (((x) > (y)) ? (x) : (y))

// 将值钳制在[min_value, max_value]范围内
// 相当于：如果a<min_value则返回min_value，a>max_value则返回max_value，否则返回a
#define SAMMI_PIN(a, min_value, max_value) SAMMI_MIN(max_value, SAMMI_MAX(a, min_value))

// 索引相关宏
////////////////////////////////////////////////////////////////////////////////

// 检查索引是否在有效范围内（左闭右开区间 [0, range)）
// 适用于数组、容器等访问验证
#define SAMMI_VALID_INDEX(idx, range) (((idx) >= 0) && ((idx) < (range)))

// 将索引强制限制在有效范围内 [0, range-1]
// 当索引越界时返回最近的有效索引
#define SAMMI_PIN_INDEX(idx, range) PICCOLO_PIN(idx, 0, (range)-1)

// 数学运算宏
////////////////////////////////////////////////////////////////////////////////

// 获取数值的符号（1.0f 表示正数，-1.0f 表示负数，0.0f 表示零）
// 注意：该实现在输入0时返回0
#define SAMMI_SIGN(x) ((((x) > 0.0f) ? 1.0f : 0.0f) + (((x) < 0.0f) ? -1.0f : 0.0f))