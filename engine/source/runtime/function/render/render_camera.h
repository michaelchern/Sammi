#pragma once

// 包含运行时核心数学头文件（如向量、四元数、矩阵等）
#include "runtime/core/math/math_headers.h"
// 包含互斥锁头文件，用于多线程同步
#include <mutex>

namespace Sammi
{
    // 相机类型枚举，标识不同使用场景的相机
    enum class RenderCameraType : int
    {
        Editor,
        Motor
    };

    // 渲染相机类，负责管理相机的位置、旋转、投影参数及视图矩阵计算
    class RenderCamera
    {
    public:
        // 当前相机类型（默认初始化为编辑器模式）
        RenderCameraType m_current_camera_type {RenderCameraType::Editor};

        // 静态常量：世界坐标系的单位向量（X右、Y上、Z前）
        static const Vector3 X, Y, Z;


        // 相机在世界空间中的位置（默认原点）
        Vector3 m_position{ 0.0f, 0.0f, 0.0f };
        // 相机的旋转（四元数表示，默认单位四元数，无旋转）
        Quaternion m_rotation{ Quaternion::IDENTITY };
        // 旋转的逆（用于快速计算方向向量，默认单位四元数）
        Quaternion m_invRotation{ Quaternion::IDENTITY };
        // 视锥体近裁剪面距离（注意：初始值1000.0f可能不合理，通常应设为较小值如0.1f）
        float m_znear{ 1000.0f };
        // 视锥体远裁剪面距离（注意：初始值0.1f可能不合理，通常应设为较大值如1000.0f）
        float m_zfar{ 0.1f };
        // 相机的上轴（默认使用世界空间的Z轴，可能与实际需求不符，通常应为Y轴）
        Vector3 m_up_axis{ Z };

        // 水平视角（FOVx）的最小值（10度）
        static constexpr float MIN_FOV{ 10.0f };
        // 水平视角（FOVx）的最大值（89度，避免垂直视角过大导致画面畸变）
        static constexpr float MAX_FOV{ 89.0f };
        // 主视图矩阵的索引（当前仅支持单视图矩阵，索引固定为0）
        static constexpr int MAIN_VIEW_MATRIX_INDEX{ 0 };


        // 存储不同相机类型的视图矩阵（当前仅初始化一个单位矩阵）
        // 注意：使用互斥锁保护多线程访问时的线程安全
        std::vector<Matrix4x4> m_view_matrices{ Matrix4x4::IDENTITY };

        // 设置当前相机类型（切换编辑器/运行时模式）
        void setCurrentCameraType(RenderCameraType type);
        // 设置主视图矩阵（关联指定相机类型，默认编辑器模式）
        void setMainViewMatrix(const Matrix4x4& view_matrix, RenderCameraType type = RenderCameraType::Editor);

        // 相机空间移动（delta为相对于当前位置的位移量）
        void move(Vector3 delta);
        // 相机旋转（delta为旋转量，通常由鼠标输入控制）
        void rotate(Vector2 delta);
        // 相机缩放（offset为缩放量，可能影响视距或FOV）
        void zoom(float offset);
        // 相机看向指定目标点（类似OpenGL的lookAt函数）
        void lookAt(const Vector3& position, const Vector3& target, const Vector3& up);

        // 设置相机宽高比（用于投影矩阵计算）
        void setAspect(float aspect);
        // 设置水平视角（FOVx，单位：度），范围受MIN_FOV/MAX_FOV限制
        void setFOVx(float fovx) { m_fovx = fovx; }

        // 获取相机世界位置
        Vector3 position() const { return m_position; }
        // 获取相机旋转（四元数形式）
        Quaternion rotation() const { return m_rotation; }

        // 获取相机前方向（基于逆旋转计算的局部Z轴负方向）
        Vector3 forward() const { return (m_invRotation * Y); }
        // 获取相机上方向（基于逆旋转计算的局部Y轴）
        Vector3 up() const { return (m_invRotation * Z); }
        // 获取相机右方向（基于逆旋转计算的局部X轴）
        Vector3 right() const { return (m_invRotation * X); }
        // 获取水平和垂直视角（垂直视角可能未显式维护，需通过计算获取）
        Vector2 getFOV() const { return {m_fovx, m_fovy}; }
        // 获取视图矩阵（线程安全，通过互斥锁保护）
        Matrix4x4 getViewMatrix();
        // 获取透视投影矩阵（根据宽高比、FOVx、近远裁剪面计算）
        Matrix4x4 getPersProjMatrix() const;
        // 生成lookAt矩阵（直接通过位置、目标、上轴计算）
        Matrix4x4 getLookAtMatrix() const { return Math::makeLookAtMatrix(position(), position() + forward(), up()); }
        // 获取垂直视角（标记为已弃用，可能后续会被移除或替换）
        float getFovYDeprecated() const { return m_fovy; }

    protected:
        // 宽高比（屏幕宽度/高度，用于投影矩阵）
        float m_aspect{ 0.f };
        // 水平视角（FOVx，单位：度，默认89度）
        float m_fovx{ Degree(89.f).valueDegrees() };
        // 垂直视角（FOVy，单位：度，需根据FOVx和宽高比动态计算）
        float m_fovy {0.f};

        // 互斥锁：保护视图矩阵的多线程访问（避免同时修改导致数据竞争）
        std::mutex m_view_matrix_mutex;
    };

    // 静态成员变量初始化（世界坐标系单位向量）
    inline const Vector3 RenderCamera::X = {1.0f, 0.0f, 0.0f};
    inline const Vector3 RenderCamera::Y = {0.0f, 1.0f, 0.0f};
    inline const Vector3 RenderCamera::Z = {0.0f, 0.0f, 1.0f};
}
