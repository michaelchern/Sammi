#pragma once

// 包含渲染实体基类和渲染数据类型定义
#include "runtime/function/render/render_entity.h"
#include "runtime/function/render/render_type.h"

namespace Sammi
{
    /// 编辑器平移坐标轴控件（继承自可渲染实体）
    class EditorTranslationAxis : public RenderEntity
    {
    public:
        EditorTranslationAxis();  // 构造函数

        // 平移坐标轴的网格数据（存储顶点、索引等信息）
        RenderMeshData m_mesh_data;
    };

    /// 编辑器旋转坐标轴控件（继承自可渲染实体）
    class EditorRotationAxis : public RenderEntity
    {
    public:
        EditorRotationAxis();  // 构造函数

        // 旋转坐标轴的网格数据
        RenderMeshData m_mesh_data;
    };

    /// 编辑器缩放坐标轴控件（继承自可渲染实体）
    class EditorScaleAxis : public RenderEntity
    {
    public:
        EditorScaleAxis();  // 构造函数

        // 缩放坐标轴的网格数据
        RenderMeshData m_mesh_data;
    };
}