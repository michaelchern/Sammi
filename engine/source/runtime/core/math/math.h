#pragma once

#include "runtime/core/math/random.h"  // 随机数相关功能

#include <algorithm>                   // 标准算法（如min、max）
#include <cmath>                       // 数学函数（sin, cos等）
#include <limits>                      // 数值范围限制（如FLT_EPSILON）

// 浮点数比较宏：判断两个浮点数是否在可接受的误差范围内相等
// 公式：|x - y| < ε * max(1, |x|, |y|)
#define CMP(x, y) (fabsf(x - y) < FLT_EPSILON * fmaxf(1.0f, fmaxf(fabsf(x), fabsf(y))))

namespace Sammi
{
    // 数学常量定义
    static const float Math_POS_INFINITY = std::numeric_limits<float>::infinity();   // 正无穷
    static const float Math_NEG_INFINITY = -std::numeric_limits<float>::infinity();  // 负无穷
    static const float Math_PI           = 3.14159265358979323846264338327950288f;   // π
    static const float Math_ONE_OVER_PI  = 1.0f / Math_PI;                           // 1/π
    static const float Math_TWO_PI       = 2.0f * Math_PI;                           // 2π
    static const float Math_HALF_PI      = 0.5f * Math_PI;                           // π/2
    static const float Math_fDeg2Rad     = Math_PI / 180.0f;                         // 角度转弧度因子
    static const float Math_fRad2Deg     = 180.0f / Math_PI;                         // 弧度转角度因子
    static const float Math_LOG2         = log(2.0f);                                // ln(2)
    static const float Math_EPSILON      = 1e-6f;                                    // 自定义小量

    static const float Float_EPSILON  = FLT_EPSILON;  // 单精度浮点数最小可识别差异
    static const float Double_EPSILON = DBL_EPSILON;  // 双精度浮点数最小可识别差异

    // 前置声明（角度和向量相关类）
    class Radian;
    class Angle;
    class Degree;

    class Vector2;
    class Vector3;
    class Vector4;
    class Matrix3x3;
    class Matrix4x4;
    class Quaternion;

    // 弧度类（表示角度以弧度为单位）
    class Radian
    {
        float m_rad;  // 弧度值

    public:
        explicit Radian(float r = 0) : m_rad(r) {}  // 显式构造函数（浮点数）
        explicit Radian(const Degree& d);           // 显式构造函数（角度制）
        Radian& operator=(float f)                  // 赋值操作符（浮点数）
        {
            m_rad = f;
            return *this;
        }
        Radian& operator=(const Degree& d);         // 赋值操作符（角度制）

        // 获取弧度值
        float valueRadians() const { return m_rad; }
        // 将弧度转换为角度（实现于文件底部）
        float valueDegrees() const;
        // 将弧度转换为当前角度单位（全局设置）的值
        float valueAngleUnits() const;

        void setValue(float f) { m_rad = f; }  // 设置弧度值

        // 一元+运算符
        const Radian& operator+() const { return *this; }
        // 加法运算符（与另一个弧度相加）
        Radian        operator+(const Radian& r) const { return Radian(m_rad + r.m_rad); }
        // 加法运算符（与角度相加）
        Radian        operator+(const Degree& d) const;
        // 加法复合赋值运算符（弧度）
        Radian&       operator+=(const Radian& r)
        {
            m_rad += r.m_rad;
            return *this;
        }
        // 加法复合赋值运算符（角度）
        Radian& operator+=(const Degree& d);
        // 一元-运算符（取负）
        Radian  operator-() const { return Radian(-m_rad); }
        // 减法运算符（弧度）
        Radian  operator-(const Radian& r) const { return Radian(m_rad - r.m_rad); }
        // 减法运算符（角度）
        Radian  operator-(const Degree& d) const;
        // 减法复合赋值运算符（弧度）
        Radian& operator-=(const Radian& r)
        {
            m_rad -= r.m_rad;
            return *this;
        }
        // 减法复合赋值运算符（角度）
        Radian& operator-=(const Degree& d);
        // 乘法运算符（标量）
        Radian  operator*(float f) const { return Radian(m_rad * f); }
        // 乘法运算符（另一个弧度——注意：这是弧度值的乘积，不是角度意义上的乘积）
        Radian  operator*(const Radian& f) const { return Radian(m_rad * f.m_rad); }
        // 乘法复合赋值运算符（标量）
        Radian& operator*=(float f)
        {
            m_rad *= f;
            return *this;
        }
        // 除法运算符（标量）
        Radian  operator/(float f) const { return Radian(m_rad / f); }
        // 除法复合赋值运算符（标量）
        Radian& operator/=(float f)
        {
            m_rad /= f;
            return *this;
        }

        // 比较运算符（与其他弧度对象比较）
        bool operator<(const Radian& r) const { return m_rad < r.m_rad; }
        bool operator<=(const Radian& r) const { return m_rad <= r.m_rad; }
        bool operator==(const Radian& r) const { return m_rad == r.m_rad; }
        bool operator!=(const Radian& r) const { return m_rad != r.m_rad; }
        bool operator>=(const Radian& r) const { return m_rad >= r.m_rad; }
        bool operator>(const Radian& r) const { return m_rad > r.m_rad; }
    };


    // 角度类（表示角度以度为单位）
    class Degree
    {
        float m_deg; // 角度值

    public:
        explicit Degree(float d = 0) : m_deg(d) {}                     // 显式构造函数（浮点数）
        explicit Degree(const Radian& r) : m_deg(r.valueDegrees()) {}  // 显式构造函数（弧度）
        Degree& operator=(float f)                                     // 赋值操作符（浮点数）
        {
            m_deg = f;
            return *this;
        }
        Degree& operator=(const Degree& d) = default;                  // 默认拷贝赋值
        Degree& operator=(const Radian& r)                             // 赋值操作符（弧度）
        {
            m_deg = r.valueDegrees();
            return *this;
        }

        // 获取角度值
        float valueDegrees() const { return m_deg; }
        // 将角度转换为弧度（实现于文件底部）
        float valueRadians() const;
        // 将角度转换为当前角度单位（全局设置）的值
        float valueAngleUnits() const;

        // 一元+运算符
        const Degree& operator+() const { return *this; }
        // 加法运算符（角度）
        Degree        operator+(const Degree& d) const { return Degree(m_deg + d.m_deg); }
        // 加法运算符（弧度）
        Degree        operator+(const Radian& r) const { return Degree(m_deg + r.valueDegrees()); }
        // 加法复合赋值运算符（角度）
        Degree&       operator+=(const Degree& d)
        {
            m_deg += d.m_deg;
            return *this;
        }
        // 加法复合赋值运算符（弧度）
        Degree& operator+=(const Radian& r)
        {
            m_deg += r.valueDegrees();
            return *this;
        }
        // 一元-运算符（取负）
        Degree  operator-() const { return Degree(-m_deg); }
        // 减法运算符（角度）
        Degree  operator-(const Degree& d) const { return Degree(m_deg - d.m_deg); }
        // 减法运算符（弧度）
        Degree  operator-(const Radian& r) const { return Degree(m_deg - r.valueDegrees()); }
        // 减法复合赋值运算符（角度）
        Degree& operator-=(const Degree& d)
        {
            m_deg -= d.m_deg;
            return *this;
        }
        // 减法复合赋值运算符（弧度）
        Degree& operator-=(const Radian& r)
        {
            m_deg -= r.valueDegrees();
            return *this;
        }
        // 乘法运算符（标量）
        Degree  operator*(float f) const { return Degree(m_deg * f); }
        // 乘法运算符（另一个角度——注意：这是角度值的乘积，不是角度意义上的乘积）
        Degree  operator*(const Degree& f) const { return Degree(m_deg * f.m_deg); }
        // 乘法复合赋值运算符（标量）
        Degree& operator*=(float f)
        {
            m_deg *= f;
            return *this;
        }
        // 除法运算符（标量)
        Degree  operator/(float f) const { return Degree(m_deg / f); }
        // 除法复合赋值运算符（标量）
        Degree& operator/=(float f)
        {
            m_deg /= f;
            return *this;
        }

        // 比较运算符（与其他角度对象比较）
        bool operator<(const Degree& d) const { return m_deg < d.m_deg; }
        bool operator<=(const Degree& d) const { return m_deg <= d.m_deg; }
        bool operator==(const Degree& d) const { return m_deg == d.m_deg; }
        bool operator!=(const Degree& d) const { return m_deg != d.m_deg; }
        bool operator>=(const Degree& d) const { return m_deg >= d.m_deg; }
        bool operator>(const Degree& d) const { return m_deg > d.m_deg; }
    };

    // 角度单位类（使用全局设置的单位）
    class Angle
    {
        float m_angle;  // 以当前全局单位的角度值

    public:
        explicit Angle(float angle) : m_angle(angle) {}
        Angle() { m_angle = 0; }

        // 转换为弧度
        explicit operator Radian() const;
        // 转换为角度
        explicit operator Degree() const;
    };

    // 数学工具类（静态方法）
    class Math
    {
    private:
        // 角度单位枚举
        enum class AngleUnit
        {
            AU_DEGREE,  // 角度
            AU_RADIAN   // 弧度
        };

        // 全局角度单位（默认为弧度）
        static AngleUnit k_AngleUnit;

    public:
        Math();

        // 绝对值
        static float abs(float value) { return std::fabs(value); }
        // 判断是否为NaN
        static bool  isNan(float f) { return std::isnan(f); }
        // 平方
        static float sqr(float value) { return value * value; }
        // 平方根
        static float sqrt(float fValue) { return std::sqrt(fValue); }
        // 平方根倒数
        static float invSqrt(float value) { return 1.f / sqrt(value); }
        // 浮点数相等判断（带容差）
        static bool  realEqual(float a, float b, float tolerance = std::numeric_limits<float>::epsilon());
        // 将值限制在[min, max]之间
        static float clamp(float v, float min, float max) { return std::clamp(v, min, max); }
        // 获取三个浮点数中的最大值
        static float getMaxElement(float x, float y, float z) { return std::max({x, y, z}); }

        // 角度单位转换函数
        static float degreesToRadians(float degrees);     // 角度转弧度
        static float radiansToDegrees(float radians);     // 弧度转角度
        static float angleUnitsToRadians(float units);    // 当前单位转弧度
        static float radiansToAngleUnits(float radians);  // 弧度转当前单位
        static float angleUnitsToDegrees(float units);    // 当前单位转角度
        static float degreesToAngleUnits(float degrees);  // 角度转当前单位

        // 三角函数（支持弧度对象和浮点数）
        static float  sin(const Radian& rad) { return std::sin(rad.valueRadians()); }
        static float  sin(float value) { return std::sin(value); }
        static float  cos(const Radian& rad) { return std::cos(rad.valueRadians()); }
        static float  cos(float value) { return std::cos(value); }
        static float  tan(const Radian& rad) { return std::tan(rad.valueRadians()); }
        static float  tan(float value) { return std::tan(value); }
        // 反三角函数（返回弧度对象）
        static Radian acos(float value);                                                    // 反余弦
        static Radian asin(float value);                                                    // 反正弦
        static Radian atan(float value) { return Radian(std::atan(value)); }                // 反正切
        static Radian atan2(float y_v, float x_v) { return Radian(std::atan2(y_v, x_v)); }  // 二参数反正切

        // 通用最大值（支持任意类型，使用operator<）
        template<class T>
        static constexpr T max(const T A, const T B)
        {
            return std::max(A, B);
        }

        // 通用最小值（支持任意类型，使用operator<）
        template<class T>
        static constexpr T min(const T A, const T B)
        {
            return std::min(A, B);
        }

        // 三值最大值
        template<class T>
        static constexpr T max3(const T A, const T B, const T C)
        {
            return std::max({A, B, C});
        }

        // 三值最小值
        template<class T>
        static constexpr T min3(const T A, const T B, const T C)
        {
            return std::min({A, B, C});
        }

        // 创建视图矩阵（相机变换）
        // position:       相机位置
        // orientation:    相机朝向（四元数）
        // reflect_matrix: 可选的反射矩阵
        static Matrix4x4
        makeViewMatrix(const Vector3& position, const Quaternion& orientation, const Matrix4x4* reflect_matrix = nullptr);

        // 创建视图矩阵（LookAt）
        // eye_position:    相机位置
        // target_position: 目标位置
        // up_dir:          上方向
        static Matrix4x4
        makeLookAtMatrix(const Vector3& eye_position, const Vector3& target_position, const Vector3& up_dir);

        // 创建透视投影矩阵
        // fovy:   垂直视角（弧度）
        // aspect: 宽高比
        // znear:  近平面距离
        // zfar:   远平面距离
        static Matrix4x4 makePerspectiveMatrix(Radian fovy, float aspect, float znear, float zfar);

        // 创建正交投影矩阵（指定六个面）
        // left, right, bottom, top, znear, zfar: 六个面的位置
        static Matrix4x4
        makeOrthographicProjectionMatrix(float left, float right, float bottom, float top, float znear, float zfar);
        
        // 创建正交投影矩阵（归一化到[0,1]范围）
        static Matrix4x4
        makeOrthographicProjectionMatrix01(float left, float right, float bottom, float top, float znear, float zfar);
    };

    // 由于Degree类在Radian类之后定义，所以以下函数在文件底部实现

    // 用角度值构造弧度值
    inline Radian::Radian(const Degree& d) : m_rad(d.valueRadians()) {}
    // 用角度值赋值
    inline Radian& Radian::operator=(const Degree& d)
    {
        m_rad = d.valueRadians();
        return *this;
    }
    // 弧度加角度
    inline Radian Radian::operator+(const Degree& d) const { return Radian(m_rad + d.valueRadians()); }
    // 弧度复合加角度
    inline Radian& Radian::operator+=(const Degree& d)
    {
        m_rad += d.valueRadians();
        return *this;
    }
    // 弧度减角度
    inline Radian Radian::operator-(const Degree& d) const { return Radian(m_rad - d.valueRadians()); }
    // 弧度复合减角度
    inline Radian& Radian::operator-=(const Degree& d)
    {
        m_rad -= d.valueRadians();
        return *this;
    }

    // 弧度转角度
    inline float Radian::valueDegrees() const { return Math::radiansToDegrees(m_rad); }
    // 弧度转当前全局单位
    inline float Radian::valueAngleUnits() const { return Math::radiansToAngleUnits(m_rad); }

    // 角度转弧度
    inline float Degree::valueRadians() const { return Math::degreesToRadians(m_deg); }
    // 角度转当前全局单位
    inline float Degree::valueAngleUnits() const { return Math::degreesToAngleUnits(m_deg); }

    // 当前全局单位角度转弧度
    inline Angle::operator Radian() const { return Radian(Math::angleUnitsToRadians(m_angle)); }
    // 当前全局单位角度转角度
    inline Angle::operator Degree() const { return Degree(Math::angleUnitsToDegrees(m_angle)); }

    // 重载运算符（标量乘弧度）
    inline Radian operator*(float a, const Radian& b) { return Radian(a * b.valueRadians()); }
    inline Radian operator/(float a, const Radian& b) { return Radian(a / b.valueRadians()); }

    // 重载运算符（标量乘角度）
    inline Degree operator*(float a, const Degree& b) { return Degree(a * b.valueDegrees()); }
    inline Degree operator/(float a, const Degree& b) { return Degree(a / b.valueDegrees()); }
}