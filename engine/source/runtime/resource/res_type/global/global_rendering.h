#pragma once

#include "runtime/core/meta/reflection/reflection.h"
#include "runtime/core/color/color.h"
#include "runtime/core/math/vector3.h"
#include "runtime/resource/res_type/data/camera_config.h"

namespace Sammi
{
    // ============================== 天空盒辐照度贴图配置 ==============================
    REFLECTION_TYPE(SkyBoxIrradianceMap)
    CLASS(SkyBoxIrradianceMap, Fields)
    {
        REFLECTION_BODY(SkyBoxIrradianceMap);

    public:
        // 六面立方体贴图路径（对应天空盒的六个方向）
        std::string m_negative_x_map;
        std::string m_positive_x_map;
        std::string m_negative_y_map;
        std::string m_positive_y_map;
        std::string m_negative_z_map;
        std::string m_positive_z_map;
    };

    // ============================== 天空盒镜面反射贴图配置 ==============================
    REFLECTION_TYPE(SkyBoxSpecularMap)
    CLASS(SkyBoxSpecularMap, Fields)
    {
        REFLECTION_BODY(SkyBoxSpecularMap);

    public:
        std::string m_negative_x_map;
        std::string m_positive_x_map;
        std::string m_negative_y_map;
        std::string m_positive_y_map;
        std::string m_negative_z_map;
        std::string m_positive_z_map;
    };

    // ============================== 定向光配置 ==============================
    REFLECTION_TYPE(DirectionalLight)
    CLASS(DirectionalLight, Fields)
    {
        REFLECTION_BODY(DirectionalLight);

    public:
        Vector3 m_direction;
        Color   m_color;
    };

    // ============================== 全局渲染资源与配置 ==============================
    REFLECTION_TYPE(GlobalRenderingRes)
    CLASS(GlobalRenderingRes, Fields)
    {
        REFLECTION_BODY(GlobalRenderingRes);

    public:
        bool                m_enable_fxaa {false};    // 是否启用FXAA抗锯齿（快速近似抗锯齿技术）
        SkyBoxIrradianceMap m_skybox_irradiance_map;  // 天空盒辐照度贴图配置（用于环境光照计算）
        SkyBoxSpecularMap   m_skybox_specular_map;    // 天空盒镜面反射贴图配置（用于PBR镜面反射）
        std::string         m_brdf_map;               // BRDF贴图路径（双向反射分布函数贴图，用于PBR计算）
        std::string         m_color_grading_map;      // 颜色分级贴图路径（用于后期处理调整色彩风格）

        Color            m_sky_color;                 // 天空盒基础颜色（当无贴图时使用的纯色）
        Color            m_ambient_light;             // 环境光颜色与强度（全局间接光照的近似）
        CameraConfig     m_camera_config;             // 相机基础配置（如视野、近远裁剪面等）
        DirectionalLight m_directional_light;         // 场景主定向光源配置（如太阳光）
    };
}
