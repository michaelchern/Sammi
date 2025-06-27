
#include "runtime/core/math/matrix3.h"

namespace Sammi
{
    // 常量定义：零矩阵和单位矩阵
    const Matrix3x3 Matrix3x3::ZERO(0, 0, 0, 0, 0, 0, 0, 0, 0);
    const Matrix3x3 Matrix3x3::IDENTITY(1, 0, 0, 0, 1, 0, 0, 0, 1);

    //-----------------------------------------------------------------------
    // 设置矩阵的指定列
    // 参数：col_index - 列索引（0,1,2），vec - 要设置的列向量
    void Matrix3x3::setColumn(size_t col_index, const Vector3& vec)
    {
        m_mat[0][col_index] = vec.x;  // 设置第一行对应列的元素
        m_mat[1][col_index] = vec.y;  // 设置第二行对应列的元素
        m_mat[2][col_index] = vec.z;  // 设置第三行对应列的元素
    }

    //-----------------------------------------------------------------------
    // 通过三个正交轴向量构建矩阵
    // 参数：x_axis - X轴向量，y_axis - Y轴向量，z_axis - Z轴向量
    void Matrix3x3::fromAxes(const Vector3& x_axis, const Vector3& y_axis, const Vector3& z_axis)
    {
        setColumn(0, x_axis);  // 第一列为X轴
        setColumn(1, y_axis);  // 第二列为Y轴
        setColumn(2, z_axis);  // 第三列为Z轴
    }

    //-----------------------------------------------------------------------
    // 计算矩阵的QDU分解（正交-对角-上三角分解）
    // 分解为：旋转矩阵(Q)、缩放矩阵(D)、错切矩阵(U)
    // 参数：out_Q - 输出的正交矩阵，out_D - 输出的缩放向量，out_U - 输出的错切向量
    void Matrix3x3::calculateQDUDecomposition(Matrix3x3& out_Q, Vector3& out_D, Vector3& out_U) const
    {
        // 使用Gram-Schmidt正交化过程计算Q矩阵
        // 第一步：归一化第一列作为Q的第一列
        float inv_length = m_mat[0][0] * m_mat[0][0] + m_mat[1][0] * m_mat[1][0] + m_mat[2][0] * m_mat[2][0];
        if (!Math::realEqual(inv_length, 0))
            inv_length = Math::invSqrt(inv_length);

        out_Q[0][0] = m_mat[0][0] * inv_length;
        out_Q[1][0] = m_mat[1][0] * inv_length;
        out_Q[2][0] = m_mat[2][0] * inv_length;

        // 第二步：计算Q的第二列（减去与第一列的投影分量后归一化）
        float dot   = out_Q[0][0] * m_mat[0][1] + out_Q[1][0] * m_mat[1][1] + out_Q[2][0] * m_mat[2][1];
        out_Q[0][1] = m_mat[0][1] - dot * out_Q[0][0];
        out_Q[1][1] = m_mat[1][1] - dot * out_Q[1][0];
        out_Q[2][1] = m_mat[2][1] - dot * out_Q[2][0];
        inv_length  = out_Q[0][1] * out_Q[0][1] + out_Q[1][1] * out_Q[1][1] + out_Q[2][1] * out_Q[2][1];
        if (!Math::realEqual(inv_length, 0))
            inv_length = Math::invSqrt(inv_length);

        out_Q[0][1] *= inv_length;
        out_Q[1][1] *= inv_length;
        out_Q[2][1] *= inv_length;

        // 第三步：计算Q的第三列（减去与前两列的投影分量后归一化）
        dot         = out_Q[0][0] * m_mat[0][2] + out_Q[1][0] * m_mat[1][2] + out_Q[2][0] * m_mat[2][2];
        out_Q[0][2] = m_mat[0][2] - dot * out_Q[0][0];
        out_Q[1][2] = m_mat[1][2] - dot * out_Q[1][0];
        out_Q[2][2] = m_mat[2][2] - dot * out_Q[2][0];
        dot         = out_Q[0][1] * m_mat[0][2] + out_Q[1][1] * m_mat[1][2] + out_Q[2][1] * m_mat[2][2];
        out_Q[0][2] -= dot * out_Q[0][1];
        out_Q[1][2] -= dot * out_Q[1][1];
        out_Q[2][2] -= dot * out_Q[2][1];
        inv_length = out_Q[0][2] * out_Q[0][2] + out_Q[1][2] * out_Q[1][2] + out_Q[2][2] * out_Q[2][2];
        if (!Math::realEqual(inv_length, 0))
            inv_length = Math::invSqrt(inv_length);

        out_Q[0][2] *= inv_length;
        out_Q[1][2] *= inv_length;
        out_Q[2][2] *= inv_length;

        // 确保正交矩阵行列式为1（无反射变换）
        float det = out_Q[0][0] * out_Q[1][1] * out_Q[2][2] + out_Q[0][1] * out_Q[1][2] * out_Q[2][0] +
                    out_Q[0][2] * out_Q[1][0] * out_Q[2][1] - out_Q[0][2] * out_Q[1][1] * out_Q[2][0] -
                    out_Q[0][1] * out_Q[1][0] * out_Q[2][2] - out_Q[0][0] * out_Q[1][2] * out_Q[2][1];

        if (det < 0.0)
        {
            // 行列式为负时翻转所有元素符号
            for (size_t row_index = 0; row_index < 3; row_index++)
                for (size_t rol_index = 0; rol_index < 3; rol_index++)
                    out_Q[row_index][rol_index] = -out_Q[row_index][rol_index];
        }

        // 计算上三角矩阵R（R = Q^T * M）
        Matrix3x3 R;
        R[0][0] = out_Q[0][0] * m_mat[0][0] + out_Q[1][0] * m_mat[1][0] + out_Q[2][0] * m_mat[2][0];
        R[0][1] = out_Q[0][0] * m_mat[0][1] + out_Q[1][0] * m_mat[1][1] + out_Q[2][0] * m_mat[2][1];
        R[1][1] = out_Q[0][1] * m_mat[0][1] + out_Q[1][1] * m_mat[1][1] + out_Q[2][1] * m_mat[2][1];
        R[0][2] = out_Q[0][0] * m_mat[0][2] + out_Q[1][0] * m_mat[1][2] + out_Q[2][0] * m_mat[2][2];
        R[1][2] = out_Q[0][1] * m_mat[0][2] + out_Q[1][1] * m_mat[1][2] + out_Q[2][1] * m_mat[2][2];
        R[2][2] = out_Q[0][2] * m_mat[0][2] + out_Q[1][2] * m_mat[1][2] + out_Q[2][2] * m_mat[2][2];

        // 提取对角缩放分量（D矩阵对角线）
        out_D[0] = R[0][0];
        out_D[1] = R[1][1];
        out_D[2] = R[2][2];

        // 计算错切分量（U矩阵的非对角线元素）
        float inv_d0 = 1.0f / out_D[0];
        out_U[0]     = R[0][1] * inv_d0;    // U01
        out_U[1]     = R[0][2] * inv_d0;    // U02
        out_U[2]     = R[1][2] / out_D[1];  // U12
    }

    //-----------------------------------------------------------------------
    // 从旋转矩阵提取旋转轴和旋转角度
    // 参数：axis - 输出的旋转轴向量，radian - 输出的旋转弧度
    void Matrix3x3::toAngleAxis(Vector3& axis, Radian& radian) const
    {
        // 计算旋转角度（通过矩阵的迹）
        float trace = m_mat[0][0] + m_mat[1][1] + m_mat[2][2];
        float cos_v = 0.5f * (trace - 1.0f);
        radian      = Math::acos(cos_v);  // 角度范围[0, PI]

        if (radian > Radian(0.0))
        {
            if (radian < Radian(Math_PI))
            {
                // 常规情况：通过反对称矩阵分量计算旋转轴
                axis.x = m_mat[2][1] - m_mat[1][2];  // Ryz - Rzy
                axis.y = m_mat[0][2] - m_mat[2][0];  // Rzx - Rxz
                axis.z = m_mat[1][0] - m_mat[0][1];  // Rxy - Ryx
                axis.normalise();  // 归一化轴向量
            }
            else
            {
                // 处理角度为PI的特殊情况（旋转180度）
                float half_inv;
                if (m_mat[0][0] >= m_mat[1][1])
                {
                    if (m_mat[0][0] >= m_mat[2][2])
                    {
                        // X轴分量最大
                        axis.x   = 0.5f * Math::sqrt(m_mat[0][0] - m_mat[1][1] - m_mat[2][2] + 1.0f);
                        half_inv = 0.5f / axis.x;
                        axis.y   = half_inv * m_mat[0][1];  // 从矩阵元素恢复Y分量
                        axis.z   = half_inv * m_mat[0][2];  // 从矩阵元素恢复Z分量
                    }
                    else
                    {
                        // Z轴分量最大
                        axis.z   = 0.5f * Math::sqrt(m_mat[2][2] - m_mat[0][0] - m_mat[1][1] + 1.0f);
                        half_inv = 0.5f / axis.z;
                        axis.x   = half_inv * m_mat[0][2];  // 从矩阵元素恢复X分量
                        axis.y   = half_inv * m_mat[1][2];  // 从矩阵元素恢复Y分量
                    }
                }
                else
                {
                    if (m_mat[1][1] >= m_mat[2][2])
                    {
                        // Y轴分量最大
                        axis.y   = 0.5f * Math::sqrt(m_mat[1][1] - m_mat[0][0] - m_mat[2][2] + 1.0f);
                        half_inv = 0.5f / axis.y;
                        axis.x   = half_inv * m_mat[0][1];  // 从矩阵元素恢复X分量
                        axis.z   = half_inv * m_mat[1][2];  // 从矩阵元素恢复Z分量
                    }
                    else
                    {
                        // Z轴分量最大
                        axis.z   = 0.5f * Math::sqrt(m_mat[2][2] - m_mat[0][0] - m_mat[1][1] + 1.0f);
                        half_inv = 0.5f / axis.z;
                        axis.x   = half_inv * m_mat[0][2];  // 从矩阵元素恢复X分量
                        axis.y   = half_inv * m_mat[1][2];  // 从矩阵元素恢复Y分量
                    }
                }
            }
        }
        else
        {
            // 角度为0（单位矩阵），返回默认X轴
            axis.x = 1.0;
            axis.y = 0.0;
            axis.z = 0.0;
        }
    }

    //-----------------------------------------------------------------------
    // 从旋转轴和角度构建旋转矩阵
    // 参数：axis - 旋转轴向量（需归一化），radian - 旋转弧度
    void Matrix3x3::fromAngleAxis(const Vector3& axis, const Radian& radian)
    {
        float cos_v         = Math::cos(radian);
        float sin_v         = Math::sin(radian);
        float one_minus_cos = 1.0f - cos_v;
        float x2            = axis.x * axis.x;
        float y2            = axis.y * axis.y;
        float z2            = axis.z * axis.z;
        float xym           = axis.x * axis.y * one_minus_cos;
        float xzm           = axis.x * axis.z * one_minus_cos;
        float yzm           = axis.y * axis.z * one_minus_cos;
        float x_sin_v       = axis.x * sin_v;
        float y_sin_v       = axis.y * sin_v;
        float z_sinv        = axis.z * sin_v;

        // 使用罗德里格斯旋转公式构建矩阵
        m_mat[0][0] = x2 * one_minus_cos + cos_v;  
        m_mat[0][1] = xym - z_sinv;                // 旋转项：xy(1-cosθ) - zsinθ
        m_mat[0][2] = xzm + y_sin_v;               // 旋转项：xz(1-cosθ) + ysinθ
        m_mat[1][0] = xym + z_sinv;                // 旋转项：xy(1-cosθ) + zsinθ
        m_mat[1][1] = y2 * one_minus_cos + cos_v;
        m_mat[1][2] = yzm - x_sin_v;               // 旋转项：yz(1-cosθ) - xsinθ
        m_mat[2][0] = xzm - y_sin_v;               // 旋转项：xz(1-cosθ) - ysinθ
        m_mat[2][1] = yzm + x_sin_v;               // 旋转项：yz(1-cosθ) + xsinθ
        m_mat[2][2] = z2 * one_minus_cos + cos_v;
    }
}