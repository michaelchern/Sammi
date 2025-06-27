#pragma once

// 包含必要的数学库
#include "runtime/core/math/math.h"
#include "runtime/core/math/quaternion.h"
#include "runtime/core/math/vector3.h"
#include <cstring>  // 用于memcpy函数

// 注意：所有代码改编自Wild Magic 0.2矩阵数学库（免费源代码）
// http://www.geometrictools.com/

// 坐标系说明：
// 假设(x,y,z)坐标系是右手坐标系
// 坐标轴旋转矩阵形式：
//   RX = [1  0      0     ]  绕X轴旋转（在yz平面逆时针）
//        [0  cos(t) -sin(t)]
//        [0  sin(t)  cos(t)]
//
//   RY = [cos(t)  0  sin(t)]  绕Y轴旋转（在zx平面逆时针）
//        [0       1  0     ]
//        [-sin(t) 0  cos(t)]
//
//   RZ = [cos(t) -sin(t) 0]  绕Z轴旋转（在xy平面逆时针）
//        [sin(t)  cos(t)  0]
//        [0       0       1]

namespace Sammi
{
    /**
     * 3x3矩阵类，用于表示三维空间中的线性变换（如旋转、缩放）
     * @note 代码改编自Wild Magic 0.2矩阵库
     * @note 使用右手坐标系
     */
    class Matrix3x3
    {
    public:
        // 矩阵数据存储（行优先：m_mat[row][column]）
        float m_mat[3][3];

    public:
        /** 默认构造函数（初始化为单位矩阵） */
        Matrix3x3() { operator=(IDENTITY); }

        /** 通过3x3数组构造矩阵 */
        explicit Matrix3x3(float arr[3][3])
        {
            // 使用内存拷贝提高效率
            memcpy(m_mat[0], arr[0], 3 * sizeof(float));
            memcpy(m_mat[1], arr[1], 3 * sizeof(float));
            memcpy(m_mat[2], arr[2], 3 * sizeof(float));
        }

        /** 通过一维数组构造矩阵（9个元素） */
        Matrix3x3(float (&float_array)[9])
        {
            // 按行填充矩阵元素
            m_mat[0][0] = float_array[0]; m_mat[0][1] = float_array[1]; m_mat[0][2] = float_array[2];
            m_mat[1][0] = float_array[3]; m_mat[1][1] = float_array[4]; m_mat[1][2] = float_array[5];
            m_mat[2][0] = float_array[6]; m_mat[2][1] = float_array[7]; m_mat[2][2] = float_array[8];
        }

        /** 通过9个独立元素构造矩阵 */
        Matrix3x3(float entry00, float entry01, float entry02,
                  float entry10, float entry11, float entry12,
                  float entry20, float entry21, float entry22)
        {
            m_mat[0][0] = entry00; m_mat[0][1] = entry01; m_mat[0][2] = entry02;
            m_mat[1][0] = entry10; m_mat[1][1] = entry11; m_mat[1][2] = entry12;
            m_mat[2][0] = entry20; m_mat[2][1] = entry21; m_mat[2][2] = entry22;
        }

        /** 通过三个行向量构造矩阵 */
        Matrix3x3(const Vector3& row0, const Vector3& row1, const Vector3& row2)
        {
            m_mat[0][0] = row0.x; m_mat[0][1] = row0.y; m_mat[0][2] = row0.z;
            m_mat[1][0] = row1.x; m_mat[1][1] = row1.y; m_mat[1][2] = row1.z;
            m_mat[2][0] = row2.x; m_mat[2][1] = row2.y; m_mat[2][2] = row2.z;
        }

        /** 通过四元数构造旋转矩阵 */
        Matrix3x3(const Quaternion& q)
        {
            // 预计算常用值
            float yy = q.y * q.y;
            float zz = q.z * q.z;
            float xy = q.x * q.y;
            float zw = q.z * q.w;
            float xz = q.x * q.z;
            float yw = q.y * q.w;
            float xx = q.x * q.x;
            float yz = q.y * q.z;
            float xw = q.x * q.w;

            // 第一行
            m_mat[0][0] = 1 - 2 * yy - 2 * zz;
            m_mat[0][1] = 2 * xy + 2 * zw;
            m_mat[0][2] = 2 * xz - 2 * yw;

            // 第二行
            m_mat[1][0] = 2 * xy - 2 * zw;
            m_mat[1][1] = 1 - 2 * xx - 2 * zz;
            m_mat[1][2] = 2 * yz + 2 * xw;

            // 第三行
            m_mat[2][0] = 2 * xz + 2 * yw;
            m_mat[2][1] = 2 * yz - 2 * xw;
            m_mat[2][2] = 1 - 2 * xx - 2 * yy;
        }

        /** 从一维数组加载矩阵数据 */
        void fromData(float (&float_array)[9])
        {
            m_mat[0][0] = float_array[0]; m_mat[0][1] = float_array[1]; m_mat[0][2] = float_array[2];
            m_mat[1][0] = float_array[3]; m_mat[1][1] = float_array[4]; m_mat[1][2] = float_array[5];
            m_mat[2][0] = float_array[6]; m_mat[2][1] = float_array[7]; m_mat[2][2] = float_array[8];
        }

        /** 将矩阵数据导出到一维数组 */
        void toData(float (&float_array)[9]) const
        {
            float_array[0] = m_mat[0][0]; float_array[1] = m_mat[0][1]; float_array[2] = m_mat[0][2];
            float_array[3] = m_mat[1][0]; float_array[4] = m_mat[1][1]; float_array[5] = m_mat[1][2];
            float_array[6] = m_mat[2][0]; float_array[7] = m_mat[2][1]; float_array[8] = m_mat[2][2];
        }

        /** 访问矩阵行（支持mat[row][col]语法） */
        float* operator[](size_t row_index) const { return (float*)m_mat[row_index]; }

        /** 获取矩阵列向量 */
        Vector3 getColumn(size_t col_index) const
        {
            assert(0 <= col_index && col_index < 3);  // 确保列索引有效
            return Vector3(m_mat[0][col_index],
                           m_mat[1][col_index],
                           m_mat[2][col_index]);
        }

        /** 设置矩阵列向量（需在源文件实现） */
        void setColumn(size_t iCol, const Vector3& vec);

        /** 通过三个轴向量构建矩阵（需在源文件实现） */
        void fromAxes(const Vector3& x_axis, const Vector3& y_axis, const Vector3& z_axis);

        // 比较运算符
        bool operator==(const Matrix3x3& rhs) const
        {
            // 逐个元素比较
            for (size_t row_index = 0; row_index < 3; row_index++)
            {
                for (size_t col_index = 0; col_index < 3; col_index++)
                {
                    if (m_mat[row_index][col_index] != rhs.m_mat[row_index][col_index])
                        return false;
                }
            }

            return true;
        }

        bool operator!=(const Matrix3x3& rhs) const { return !operator==(rhs); }

        // 算术运算
        Matrix3x3 operator+(const Matrix3x3& rhs) const
        {
            Matrix3x3 sum;
            for (size_t row_index = 0; row_index < 3; row_index++)
            {
                for (size_t col_index = 0; col_index < 3; col_index++)
                {
                    sum.m_mat[row_index][col_index] = m_mat[row_index][col_index] + rhs.m_mat[row_index][col_index];
                }
            }
            return sum;
        }

        Matrix3x3 operator-(const Matrix3x3& rhs) const
        {
            Matrix3x3 diff;
            for (size_t row_index = 0; row_index < 3; row_index++)
            {
                for (size_t col_index = 0; col_index < 3; col_index++)
                {
                    diff.m_mat[row_index][col_index] = m_mat[row_index][col_index] - rhs.m_mat[row_index][col_index];
                }
            }
            return diff;
        }

        Matrix3x3 operator*(const Matrix3x3& rhs) const
        {
            Matrix3x3 prod;
            for (size_t row_index = 0; row_index < 3; row_index++)
            {
                for (size_t col_index = 0; col_index < 3; col_index++)
                {
                    prod.m_mat[row_index][col_index] = m_mat[row_index][0] * rhs.m_mat[0][col_index] +
                                                       m_mat[row_index][1] * rhs.m_mat[1][col_index] +
                                                       m_mat[row_index][2] * rhs.m_mat[2][col_index];
                }
            }
            return prod;
        }

        /** 矩阵乘以向量（3x3 * 3x1 = 3x1） */
        Vector3 operator*(const Vector3& rhs) const
        {
            Vector3 prod;
            for (size_t row_index = 0; row_index < 3; row_index++)
            {
                prod[row_index] =
                    m_mat[row_index][0] * rhs.x + m_mat[row_index][1] * rhs.y + m_mat[row_index][2] * rhs.z;
            }
            return prod;
        }

        /** 向量乘以矩阵（1x3 * 3x3 = 1x3） */
        friend Vector3 operator*(const Vector3& point, const Matrix3x3& rhs)
        {
            Vector3 prod;
            for (size_t row_index = 0; row_index < 3; row_index++)
            {
                prod[row_index] = point.x * rhs.m_mat[0][row_index] + point.y * rhs.m_mat[1][row_index] +
                                  point.z * rhs.m_mat[2][row_index];
            }
            return prod;
        }

        /** 矩阵取负 */
        Matrix3x3 operator-() const
        {
            Matrix3x3 neg;
            for (size_t row_index = 0; row_index < 3; row_index++)
            {
                for (size_t col_index = 0; col_index < 3; col_index++)
                    neg[row_index][col_index] = -m_mat[row_index][col_index];
            }
            return neg;
        }

        /** 矩阵乘以标量 */
        Matrix3x3 operator*(float scalar) const
        {
            Matrix3x3 prod;
            for (size_t row_index = 0; row_index < 3; row_index++)
            {
                for (size_t col_index = 0; col_index < 3; col_index++)
                    prod[row_index][col_index] = scalar * m_mat[row_index][col_index];
            }
            return prod;
        }

        /** 标量乘以矩阵 */
        friend Matrix3x3 operator*(float scalar, const Matrix3x3& rhs)
        {
            Matrix3x3 prod;
            for (size_t row_index = 0; row_index < 3; row_index++)
            {
                for (size_t col_index = 0; col_index < 3; col_index++)
                    prod[row_index][col_index] = scalar * rhs.m_mat[row_index][col_index];
            }
            return prod;
        }

        // 矩阵操作
        /** 转置矩阵 */
        Matrix3x3 transpose() const
        {
            Matrix3x3 transpose_v;
            for (size_t row_index = 0; row_index < 3; row_index++)
            {
                for (size_t col_index = 0; col_index < 3; col_index++)
                    transpose_v[row_index][col_index] = m_mat[col_index][row_index];
            }
            return transpose_v;
        }

        /** 矩阵求逆（使用伴随矩阵法） */
        bool inverse(Matrix3x3& inv_mat, float fTolerance = 1e-06) const
        {
            // 计算行列式
            float det = determinant();
            if (std::fabs(det) <= fTolerance)
                return false;  // 奇异矩阵不可逆

            // 计算余子式矩阵（伴随矩阵）
            inv_mat[0][0] = m_mat[1][1] * m_mat[2][2] - m_mat[1][2] * m_mat[2][1];
            inv_mat[0][1] = m_mat[0][2] * m_mat[2][1] - m_mat[0][1] * m_mat[2][2];
            inv_mat[0][2] = m_mat[0][1] * m_mat[1][2] - m_mat[0][2] * m_mat[1][1];
            inv_mat[1][0] = m_mat[1][2] * m_mat[2][0] - m_mat[1][0] * m_mat[2][2];
            inv_mat[1][1] = m_mat[0][0] * m_mat[2][2] - m_mat[0][2] * m_mat[2][0];
            inv_mat[1][2] = m_mat[0][2] * m_mat[1][0] - m_mat[0][0] * m_mat[1][2];
            inv_mat[2][0] = m_mat[1][0] * m_mat[2][1] - m_mat[1][1] * m_mat[2][0];
            inv_mat[2][1] = m_mat[0][1] * m_mat[2][0] - m_mat[0][0] * m_mat[2][1];
            inv_mat[2][2] = m_mat[0][0] * m_mat[1][1] - m_mat[0][1] * m_mat[1][0];

            // 应用行列式倒数
            float inv_det = 1.0f / det;
            for (size_t row_index = 0; row_index < 3; row_index++)
            {
                for (size_t col_index = 0; col_index < 3; col_index++)
                    inv_mat[row_index][col_index] *= inv_det;
            }

            return true;
        }

        /** 求逆矩阵（返回结果） */
        Matrix3x3 inverse(float tolerance = 1e-06) const
        {
            Matrix3x3 inv = ZERO;
            inverse(inv, tolerance);
            return inv;
        }

        /** 计算行列式 */
        float determinant() const
        {
            float cofactor00 = m_mat[1][1] * m_mat[2][2] - m_mat[1][2] * m_mat[2][1];
            float cofactor10 = m_mat[1][2] * m_mat[2][0] - m_mat[1][0] * m_mat[2][2];
            float cofactor20 = m_mat[1][0] * m_mat[2][1] - m_mat[1][1] * m_mat[2][0];

            float det = m_mat[0][0] * cofactor00 + m_mat[0][1] * cofactor10 + m_mat[0][2] * cofactor20;

            return det;
        }

        /** QDU分解（正交-对角-上三角分解） */
        void calculateQDUDecomposition(Matrix3x3& out_Q, Vector3& out_D, Vector3& out_U) const;

        // 旋转矩阵操作
        /** 将旋转矩阵转换为轴角表示 */
        void toAngleAxis(Vector3& axis, Radian& angle) const;

        /** 将旋转矩阵转换为轴角表示（角度以度为单位） */
        void toAngleAxis(Vector3& axis, Degree& angle) const
        {
            Radian r;
            toAngleAxis(axis, r);
            angle = r;
        }

        /** 从轴角表示创建旋转矩阵 */
        void fromAngleAxis(const Vector3& axis, const Radian& radian);

        /** 创建缩放矩阵 */
        static Matrix3x3 scale(const Vector3& scale)
        {
            Matrix3x3 mat = ZERO;

            mat.m_mat[0][0] = scale.x;
            mat.m_mat[1][1] = scale.y;
            mat.m_mat[2][2] = scale.z;

            return mat;
        }

        // 静态常量
        static const Matrix3x3 ZERO;      // 零矩阵
        static const Matrix3x3 IDENTITY;  // 单位矩阵
    };
}