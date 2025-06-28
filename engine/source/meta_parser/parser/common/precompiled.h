#pragma once

// 标准库头文件
#include <algorithm>      // 算法库（排序、查找等）
#include <chrono>         // 时间库（计时、时间点等）
#include <filesystem>     // 文件系统库（路径操作）
#include <fstream>        // 文件流（读写文件）
#include <iostream>       // 输入输出流
#include <map>            // 有序映射
#include <set>            // 有序集合
#include <sstream>        // 字符串流
#include <string>         // 字符串处理
#include <unordered_map>  // 哈希映射
#include <unordered_set>  // 哈希集合
#include <vector>         // 动态数组

// Clang C接口（用于源码解析）
#include <clang-c/Index.h>  // Clang的C语言接口

// 文件系统命名空间别名
namespace fs = std::filesystem;

// 项目特定头文件
#include "meta/meta_data_config.h"  // 元数据生成配置
#include "meta/meta_utils.h"        // 元数据工具函数

// Mustache模板引擎
#include "mustache.hpp"                  // Mustache模板库
namespace Mustache = kainjow::mustache;  // 命名空间别名