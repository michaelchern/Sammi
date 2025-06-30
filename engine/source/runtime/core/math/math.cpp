#include "runtime/core/math/math.h"
#include "runtime/core/math/matrix4.h"

namespace Sammi
{
    // 静态成员初始化
    Math::AngleUnit Math::k_AngleUnit;

    // 构造函数：默认使用角度制
    Math::Math() { k_AngleUnit = AngleUnit::AU_DEGREE; }

    // 浮点数近似相等比较
    // a, b: 要比较的浮点数
    // tolerance: 允许的误差范围（默认使用机器精度）
    bool Math::realEqual(float a, float b, float tolerance /* = std::numeric_limits<float>::epsilon() */)
    {
        return std::fabs(b - a) <= tolerance;
    }

    // 角度转弧度
    float Math::degreesToRadians(float degrees) { return degrees * Math_fDeg2Rad; }

    // 弧度转角度
    float Math::radiansToDegrees(float radians) { return radians * Math_fRad2Deg; }

    // 当前角度单位转弧度
    float Math::angleUnitsToRadians(float angleunits)
    {
        if (k_AngleUnit == AngleUnit::AU_DEGREE)
            return angleunits * Math_fDeg2Rad;

        return angleunits;  // 已经是弧度
    }

    // 弧度转当前角度单位
    float Math::radiansToAngleUnits(float radians)
    {
        if (k_AngleUnit == AngleUnit::AU_DEGREE)
            return radians * Math_fRad2Deg;

        return radians;  // 已经是弧度
    }

    // 当前角度单位转角度
    float Math::angleUnitsToDegrees(float angleunits)
    {
        if (k_AngleUnit == AngleUnit::AU_RADIAN)
            return angleunits * Math_fRad2Deg;

        return angleunits;
    }

    // 角度转当前角度单位
    float Math::degreesToAngleUnits(float degrees)
    {
        if (k_AngleUnit == AngleUnit::AU_RADIAN)
            return degrees * Math_fDeg2Rad;

        return degrees;  // 已经是角度
    }

    // 安全反余弦函数（限制输入在[-1,1]范围内）
    Radian Math::acos(float value)
    {
        if (-1.0 < value)
        {
            if (value < 1.0)
                return Radian(::acos(value));  // 标准反余弦

            return Radian(0.0);  // 输入>=1时返回0弧度
        }

        return Radian(Math_PI);  // 输入<=-1时返回π弧度
    }

    // 安全反正弦函数（限制输入在[-1,1]范围内）
    Radian Math::asin(float value)
    {
        if (-1.0 < value)
        {
            if (value < 1.0)
                return Radian(::asin(value));  // 标准反正弦

            return Radian(Math_HALF_PI);  // 输入>=1时返回π/2弧度
        }

        return Radian(-Math_HALF_PI);  // 输入<=-1时返回-π/2弧度
    }

    // 创建视图矩阵（基于位置和朝向）
    // position: 相机位置
    // orientation: 相机朝向（四元数）
    // reflect_matrix: 可选的反射矩阵
    Matrix4x4 Math::makeViewMatrix(const Vector3& position, const Quaternion& orientation, const Matrix4x4* reflect_matrix)
    {
        Matrix4x4 viewMatrix;

        // 视图矩阵结构：
        // [ Rx  Uy  Fz  Tx ]
        // [ Rx  Uy  Fz  Ty ]
        // [ Rx  Uy  Fz  Tz ]
        // [ 0   0   0   1  ]
        // 其中 T = -(旋转矩阵的转置 * 位置)

        // 从四元数获取旋转矩阵
        Matrix3x3 rot;
        orientation.toRotationMatrix(rot);

        // 计算平移分量（在旋转后的坐标系中）
        Matrix3x3 rotT  = rot.transpose();
        Vector3   trans = -rotT * position;

        // 构建基础视图矩阵
        viewMatrix = Matrix4x4::IDENTITY;
        viewMatrix.setMatrix3x3(rotT);  // 设置左上3x3旋转部分
        viewMatrix[0][3] = trans.x;  // 设置平移分量
        viewMatrix[1][3] = trans.y;
        viewMatrix[2][3] = trans.z;

        // 应用反射矩阵（如存在）
        if (reflect_matrix)
        {
            viewMatrix = viewMatrix * (*reflect_matrix);
        }

        return viewMatrix;
    }

    // 创建LookAt视图矩阵
    // eye_position: 相机位置
    // target_position: 目标位置
    // up_dir: 上方向量
    Matrix4x4 Math::makeLookAtMatrix(const Vector3& eye_position, const Vector3& target_position, const Vector3& up_dir)
    {
        // 标准化上方向量
        const Vector3& up = up_dir.normalisedCopy();

        // 计算前向向量（从相机指向目标）
        Vector3 f = (target_position - eye_position).normalisedCopy();

        // 计算右向量（侧向）
        Vector3 s = f.crossProduct(up).normalisedCopy();

        // 重新计算上向量（确保正交）
        Vector3 u = s.crossProduct(f);

        // 构建视图矩阵
        Matrix4x4 view_mat = Matrix4x4::IDENTITY;

        // 设置旋转部分
        view_mat[0][0] = s.x; view_mat[0][1] = s.y; view_mat[0][2] = s.z; view_mat[0][3] = -s.dotProduct(eye_position);
        view_mat[1][0] = u.x; view_mat[1][1] = u.y; view_mat[1][2] = u.z; view_mat[1][3] = -u.dotProduct(eye_position);
        view_mat[2][0] = -f.x;view_mat[2][1] = -f.y;view_mat[2][2] = -f.z;view_mat[2][3] = f.dotProduct(eye_position);
        return view_mat;
    }

    // 创建透视投影矩阵
    // fovy: 垂直视野（弧度）
    // aspect: 宽高比
    // znear: 近平面
    // zfar: 远平面
    Matrix4x4 Math::makePerspectiveMatrix(Radian fovy, float aspect, float znear, float zfar)
    {
        // 计算半视角的正切值
        float tan_half_fovy = Math::tan(fovy / 2.f);

        // 初始化零矩阵
        Matrix4x4 ret = Matrix4x4::ZERO;

        // 设置透视参数
        ret[0][0] = 1.f / (aspect * tan_half_fovy);    // X轴缩放
        ret[1][1] = 1.f / tan_half_fovy;               // Y轴缩放
        ret[2][2] = zfar / (znear - zfar);             // Z轴深度映射
        ret[3][2] = -1.f;                              // 透视除法标记
        ret[2][3] = -(zfar * znear) / (zfar - znear);  // 深度偏移

        return ret;
    }

    // 创建正交投影矩阵（深度范围[-1,1]）
    Matrix4x4 Math::makeOrthographicProjectionMatrix(float left, float right, float bottom, float top, float znear, float zfar)
    {
        // 计算逆值
        float inv_width    = 1.0f / (right - left);
        float inv_height   = 1.0f / (top - bottom);
        float inv_distance = 1.0f / (zfar - znear);

        // 计算矩阵参数
        float A  = 2 * inv_width;                   // X轴缩放
        float B  = 2 * inv_height;                  // Y轴缩放
        float C  = -(right + left) * inv_width;     // X轴平移
        float D  = -(top + bottom) * inv_height;    // Y轴平移
        float q  = -2 * inv_distance;               // Z轴缩放
        float qn = -(zfar + znear) * inv_distance;  // Z轴平移

        // 构建正交投影矩阵
        Matrix4x4 proj_matrix = Matrix4x4::ZERO;
        proj_matrix[0][0] = A;   // X缩放
        proj_matrix[0][3] = C;   // X平移
        proj_matrix[1][1] = B;   // Y缩放
        proj_matrix[1][3] = D;   // Y平移
        proj_matrix[2][2] = q;   // Z缩放
        proj_matrix[2][3] = qn;  // Z平移
        proj_matrix[3][3] = 1;   // 齐次坐标

        return proj_matrix;
    }

    // 创建正交投影矩阵（深度范围[0,1]）
    Matrix4x4 Math::makeOrthographicProjectionMatrix01(float left, float right, float bottom, float top, float znear, float zfar)
    {
        // 计算逆值
        float inv_width    = 1.0f / (right - left);
        float inv_height   = 1.0f / (top - bottom);
        float inv_distance = 1.0f / (zfar - znear);

        // 计算矩阵参数（深度范围[0,1]）
        float A  = 2 * inv_width;                 // X轴缩放
        float B  = 2 * inv_height;                // Y轴缩放
        float C  = -(right + left) * inv_width;   // X轴平移
        float D  = -(top + bottom) * inv_height;  // Y轴平移
        float q  = -1 * inv_distance;             // Z轴缩放
        float qn = -znear * inv_distance;         // Z轴平移

        // 构建正交投影矩阵
        Matrix4x4 proj_matrix = Matrix4x4::ZERO;
        proj_matrix[0][0] = A;   // X缩放
        proj_matrix[0][3] = C;   // X平移
        proj_matrix[1][1] = B;   // Y缩放
        proj_matrix[1][3] = D;   // Y平移
        proj_matrix[2][2] = q;   // Z缩放
        proj_matrix[2][3] = qn;  // Z平移
        proj_matrix[3][3] = 1;   // 齐次坐标

        return proj_matrix;
    }
}