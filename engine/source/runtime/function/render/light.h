#pragma once

#include "runtime/core/math/vector3.h"
#include "runtime/function/render/render_type.h"

#include <vector>

namespace Sammi
{
    // 点光源结构体：表示场景中的一个点光源
    struct PointLight
    {
        // 光源在世界空间中的位置（三维坐标）
        Vector3 m_position;

        // 辐射通量（单位：瓦特W）：表示光源向整个空间发射的总光功率
        // 注：辐射通量是光源的能量输出总量，区别于辐照度（单位面积的功率）
        Vector3 m_flux;

        // 计算光照剔除的合适半径（用于优化渲染性能）
        // 原理：找到一个半径，使得在此半径外的区域光照衰减至低于指定阈值（避免不必要的渲染计算）
        // 注：此方法为经验公式，非物理精确计算，通常由美术人员调整参数控制效果
        float calculateRadius() const
        {
            // 光强衰减截止阈值（W/m²）：低于此值认为无可见光照
            const float INTENSITY_CUTOFF = 1.0f;
            // 衰减系数辅助阈值：用于平滑过渡
            const float ATTENTUATION_CUTOFF = 0.05f;
            // 计算光强（辐射强度）：光通量均匀分布在球面上，强度 = 总通量 / (4πr²)，此处r=1，故直接除以4π
            Vector3 intensity = m_flux / (4.0f * Math_PI);
            // 取光强各分量（x/y/z方向）的最大值（光源可能非各向同性）
            float maxIntensity = Vector3::getMaxElement(intensity);
            // 计算衰减系数：取INTENSITY_CUTOFF和（ATTENTUATION_CUTOFF*maxIntensity）中的较大值，再归一化到maxIntensity
            // 注：此公式确保在maxIntensity较大时，衰减主要由INTENSITY_CUTOFF决定；较小时由ATTENTUATION_CUTOFF控制
            float attenuation = Math::max(INTENSITY_CUTOFF, ATTENTUATION_CUTOFF * maxIntensity) / maxIntensity;
            // 半径与衰减系数的平方根成反比（推导：衰减公式通常为1/(k*r²)，当衰减=attenuation时，r=1/√(k*attenuation)，此处k=1）
            return 1.0f / sqrtf(attenuation);
        }
    };

    // 环境光结构体：表示场景中的全局环境光照
    struct AmbientLight
    {
        // 辐照度（单位：W/m²）：表示环境中各方向的平均入射光功率密度
        Vector3 m_irradiance;
    };

    // 平行光结构体（可能为"Parallel Directional Light"的缩写，P表示某种特性如投影）
    struct PDirectionalLight
    {
        // 光照方向（单位向量，通常指向光源相反方向，如太阳光方向）
        Vector3 m_direction;
        // 光的颜色（RGB分量，通常已归一化或包含亮度信息）
        Vector3 m_color;
    };

    // 光源列表基类结构体：定义通用光源数据格式（主要为GPU缓冲区服务）
    struct LightList
    {
        // 点光源顶点结构体：适配GPU顶点缓冲区的内存布局（需满足16字节对齐）
        struct PointLightVertex
        {
            // 光源位置（与PointLight中的m_position同步）
            Vector3 m_position;

            // 填充字段（确保后续成员从16字节对齐的地址开始）
            float m_padding;

            // 辐射强度（单位：W/sr）：表示光源在单位立体角内发射的功率（与m_flux的关系：强度= m_flux / (4π)）
            // 注：辐射强度是描述点光源方向性发光的关键参数（各向同性光源各方向强度相同）
            Vector3 m_intensity;

            // 光照有效半径（由calculateRadius计算，用于剔除不可见区域）
            float m_radius;
        };
    };

    // 点光源列表类：继承自LightList，具体管理点光源数据及GPU上传
    class PointLightList : public LightList
    {
    public:
        // 初始化函数（预留接口，当前为空实现）
        // 用途：可能用于创建GPU缓冲区、初始化默认光源等
        void init() {}

        // 关闭函数（预留接口，当前为空实现）
        // 用途：释放GPU资源、清理内存等
        void shutdown() {}

        // 更新函数（预留接口，当前为空实现）
        // 用途：将CPU端光源数据同步到GPU缓冲区（如位置、颜色、半径变化时调用）
        void update() {}

        // CPU端存储的点光源数据（动态数组，可随时增删改）
        std::vector<PointLight> m_lights;

        // 指向GPU缓冲区的共享指针（存储PointLightVertex格式的数据，供着色器访问）
        std::shared_ptr<BufferData> m_buffer;
    };
}
