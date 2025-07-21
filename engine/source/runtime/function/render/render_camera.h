#pragma once

// ��������ʱ������ѧͷ�ļ�������������Ԫ��������ȣ�
#include "runtime/core/math/math_headers.h"
// ����������ͷ�ļ������ڶ��߳�ͬ��
#include <mutex>

namespace Sammi
{
    // �������ö�٣���ʶ��ͬʹ�ó��������
    enum class RenderCameraType : int
    {
        Editor,
        Motor
    };

    // ��Ⱦ����࣬������������λ�á���ת��ͶӰ��������ͼ�������
    class RenderCamera
    {
    public:
        // ��ǰ������ͣ�Ĭ�ϳ�ʼ��Ϊ�༭��ģʽ��
        RenderCameraType m_current_camera_type {RenderCameraType::Editor};

        // ��̬��������������ϵ�ĵ�λ������X�ҡ�Y�ϡ�Zǰ��
        static const Vector3 X, Y, Z;


        // ���������ռ��е�λ�ã�Ĭ��ԭ�㣩
        Vector3 m_position{ 0.0f, 0.0f, 0.0f };
        // �������ת����Ԫ����ʾ��Ĭ�ϵ�λ��Ԫ��������ת��
        Quaternion m_rotation{ Quaternion::IDENTITY };
        // ��ת���棨���ڿ��ټ��㷽��������Ĭ�ϵ�λ��Ԫ����
        Quaternion m_invRotation{ Quaternion::IDENTITY };
        // ��׶����ü�����루ע�⣺��ʼֵ1000.0f���ܲ�����ͨ��Ӧ��Ϊ��Сֵ��0.1f��
        float m_znear{ 1000.0f };
        // ��׶��Զ�ü�����루ע�⣺��ʼֵ0.1f���ܲ�����ͨ��Ӧ��Ϊ�ϴ�ֵ��1000.0f��
        float m_zfar{ 0.1f };
        // ��������ᣨĬ��ʹ������ռ��Z�ᣬ������ʵ�����󲻷���ͨ��ӦΪY�ᣩ
        Vector3 m_up_axis{ Z };

        // ˮƽ�ӽǣ�FOVx������Сֵ��10�ȣ�
        static constexpr float MIN_FOV{ 10.0f };
        // ˮƽ�ӽǣ�FOVx�������ֵ��89�ȣ����ⴹֱ�ӽǹ����»�����䣩
        static constexpr float MAX_FOV{ 89.0f };
        // ����ͼ�������������ǰ��֧�ֵ���ͼ���������̶�Ϊ0��
        static constexpr int MAIN_VIEW_MATRIX_INDEX{ 0 };


        // �洢��ͬ������͵���ͼ���󣨵�ǰ����ʼ��һ����λ����
        // ע�⣺ʹ�û������������̷߳���ʱ���̰߳�ȫ
        std::vector<Matrix4x4> m_view_matrices{ Matrix4x4::IDENTITY };

        // ���õ�ǰ������ͣ��л��༭��/����ʱģʽ��
        void setCurrentCameraType(RenderCameraType type);
        // ��������ͼ���󣨹���ָ��������ͣ�Ĭ�ϱ༭��ģʽ��
        void setMainViewMatrix(const Matrix4x4& view_matrix, RenderCameraType type = RenderCameraType::Editor);

        // ����ռ��ƶ���deltaΪ����ڵ�ǰλ�õ�λ������
        void move(Vector3 delta);
        // �����ת��deltaΪ��ת����ͨ�������������ƣ�
        void rotate(Vector2 delta);
        // ������ţ�offsetΪ������������Ӱ���Ӿ��FOV��
        void zoom(float offset);
        // �������ָ��Ŀ��㣨����OpenGL��lookAt������
        void lookAt(const Vector3& position, const Vector3& target, const Vector3& up);

        // ���������߱ȣ�����ͶӰ������㣩
        void setAspect(float aspect);
        // ����ˮƽ�ӽǣ�FOVx����λ���ȣ�����Χ��MIN_FOV/MAX_FOV����
        void setFOVx(float fovx) { m_fovx = fovx; }

        // ��ȡ�������λ��
        Vector3 position() const { return m_position; }
        // ��ȡ�����ת����Ԫ����ʽ��
        Quaternion rotation() const { return m_rotation; }

        // ��ȡ���ǰ���򣨻�������ת����ľֲ�Z�Ḻ����
        Vector3 forward() const { return (m_invRotation * Y); }
        // ��ȡ����Ϸ��򣨻�������ת����ľֲ�Y�ᣩ
        Vector3 up() const { return (m_invRotation * Z); }
        // ��ȡ����ҷ��򣨻�������ת����ľֲ�X�ᣩ
        Vector3 right() const { return (m_invRotation * X); }
        // ��ȡˮƽ�ʹ�ֱ�ӽǣ���ֱ�ӽǿ���δ��ʽά������ͨ�������ȡ��
        Vector2 getFOV() const { return {m_fovx, m_fovy}; }
        // ��ȡ��ͼ�����̰߳�ȫ��ͨ��������������
        Matrix4x4 getViewMatrix();
        // ��ȡ͸��ͶӰ���󣨸��ݿ�߱ȡ�FOVx����Զ�ü�����㣩
        Matrix4x4 getPersProjMatrix() const;
        // ����lookAt����ֱ��ͨ��λ�á�Ŀ�ꡢ������㣩
        Matrix4x4 getLookAtMatrix() const { return Math::makeLookAtMatrix(position(), position() + forward(), up()); }
        // ��ȡ��ֱ�ӽǣ����Ϊ�����ã����ܺ����ᱻ�Ƴ����滻��
        float getFovYDeprecated() const { return m_fovy; }

    protected:
        // ��߱ȣ���Ļ���/�߶ȣ�����ͶӰ����
        float m_aspect{ 0.f };
        // ˮƽ�ӽǣ�FOVx����λ���ȣ�Ĭ��89�ȣ�
        float m_fovx{ Degree(89.f).valueDegrees() };
        // ��ֱ�ӽǣ�FOVy����λ���ȣ������FOVx�Ϳ�߱ȶ�̬���㣩
        float m_fovy {0.f};

        // ��������������ͼ����Ķ��̷߳��ʣ�����ͬʱ�޸ĵ������ݾ�����
        std::mutex m_view_matrix_mutex;
    };

    // ��̬��Ա������ʼ������������ϵ��λ������
    inline const Vector3 RenderCamera::X = {1.0f, 0.0f, 0.0f};
    inline const Vector3 RenderCamera::Y = {0.0f, 1.0f, 0.0f};
    inline const Vector3 RenderCamera::Z = {0.0f, 0.0f, 1.0f};
}
