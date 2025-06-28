#include "editor/include/axis.h"  // 包含坐标轴控件头文件

namespace Sammi
{
    // 平移坐标轴构造函数：创建平移轴几何体
    EditorTranslationAxis::EditorTranslationAxis()
    {
        // 创建平移轴渲染网格
        const float radius = 0.031f;  // 圆柱半径
        const int   segments = 12;    // 圆柱分段数（影响平滑度）

        uint32_t stride = sizeof(MeshVertexDataDefinition);  // 顶点数据结构大小

        // 顶点缓冲区分配（3个圆柱段 + 2个箭头顶点）× 3个轴
        size_t vertex_data_size = (3 * segments + 2) * 3 * stride;
        m_mesh_data.m_static_mesh_data.m_vertex_buffer = std::make_shared<BufferData>(vertex_data_size);
        uint8_t* vertex_data = static_cast<uint8_t*>(m_mesh_data.m_static_mesh_data.m_vertex_buffer->m_data);

        // X轴几何构造 ======================================================
        // 起点圆柱（X轴起点）
        for (int i = 0; i < segments; ++i)
        {
            MeshVertexDataDefinition& vertex = *(MeshVertexDataDefinition*)(vertex_data + (0 * segments + i) * stride);
            vertex.x = 0.0f;  // 起点在原点
            vertex.y = sin(i * 2 * Math_PI / segments) * radius;  // Y坐标
            vertex.z = cos(i * 2 * Math_PI / segments) * radius;  // Z坐标
            vertex.u = 0.0f;  // UV坐标U分量（用于颜色区分）

            vertex.nx = vertex.ny = vertex.nz = 0.0f;
            vertex.tx = vertex.ty = vertex.tz = 0.0f;
            vertex.v = 0.0f;
        }

        // 终点圆柱（X轴主体）
        for (int i = 0; i < segments; ++i)
        {
            MeshVertexDataDefinition& vertex = *(MeshVertexDataDefinition*)(vertex_data + (1 * segments + i) * stride);
            vertex.x = 1.5f;  // X轴长度
            vertex.y = (*(MeshVertexDataDefinition*)(vertex_data + (0 * segments + i) * stride)).y;  // 保持Y坐标
            vertex.z = (*(MeshVertexDataDefinition*)(vertex_data + (0 * segments + i) * stride)).z;  // 保持Z坐标
            vertex.u = 0.0f;

            vertex.nx = vertex.ny = vertex.nz = 0.0f;
            vertex.tx = vertex.ty = vertex.tz = 0.0f;
            vertex.v = 0.0f;
        }

        // 箭头锥体（X轴末端）
        for (int i = 0; i < segments; ++i)
        {
            MeshVertexDataDefinition& vertex = *(MeshVertexDataDefinition*)(vertex_data + (2 * segments + i) * stride);
            vertex.x = 1.5f;  // 与主体末端对齐
            vertex.y = (*(MeshVertexDataDefinition*)(vertex_data + (0 * segments + i) * stride)).y * 4.5f;
            vertex.z = (*(MeshVertexDataDefinition*)(vertex_data + (0 * segments + i) * stride)).z * 4.5f;
            vertex.u = 0.0f;

            vertex.nx = vertex.ny = vertex.nz = 0.0f;
            vertex.tx = vertex.ty = vertex.tz = 0.0f;
            vertex.v = 0.0f;
        }

        // 箭头连接点（X轴）
        {
            // 箭头底部中心点
            MeshVertexDataDefinition& vertex_0 = *(MeshVertexDataDefinition*)(vertex_data + (3 * segments + 0) * stride);
            vertex_0.x = 1.5f;
            vertex_0.y = 0.0f;
            vertex_0.z = 0.0f;
            vertex_0.u = 0.0f;

            vertex_0.nx = vertex_0.ny = vertex_0.nz = 0.0f;
            vertex_0.tx = vertex_0.ty = vertex_0.tz = 0.0f;
            vertex_0.v = 0.0f;

            // 箭头尖端
            MeshVertexDataDefinition& vertex_1 = *(MeshVertexDataDefinition*)(vertex_data + (3 * segments + 1) * stride);
            vertex_1.x = 1.9f;
            vertex_1.y = 0.0f;
            vertex_1.z = 0.0f;
            vertex_1.u = 0.0f;

            vertex_1.nx = vertex_1.ny = vertex_1.nz = 0.0f;
            vertex_1.tx = vertex_1.ty = vertex_1.tz = 0.0f;
            vertex_1.v = 0.0f;
        }

        // Y轴和Z轴构造（通过旋转X轴几何）====================================
        for (int i = 0; i < 3 * segments + 2; ++i)
        {
            // Y轴：将X轴几何绕Z轴旋转90度
            MeshVertexDataDefinition& vertex_y = *(MeshVertexDataDefinition*)(vertex_data + ((3 * segments + 2) * 1 + i) * stride);
            vertex_y.x = -(*(MeshVertexDataDefinition*)(vertex_data + i * stride)).y;  // 交换X和Y
            vertex_y.y = (*(MeshVertexDataDefinition*)(vertex_data + i * stride)).x;
            vertex_y.z = (*(MeshVertexDataDefinition*)(vertex_data + i * stride)).z;
            vertex_y.u = 1.0f;  // 使用不同UV区分颜色

            vertex_y.nx = vertex_y.ny = vertex_y.nz = 0.0f;
            vertex_y.tx = vertex_y.ty = vertex_y.tz = 0.0f;
            vertex_y.v = 0.0f;

            // Z轴：将X轴几何绕Y轴旋转90度
            MeshVertexDataDefinition& vertex_z = *(MeshVertexDataDefinition*)(vertex_data + ((3 * segments + 2) * 2 + i) * stride);
            vertex_z.x = -(*(MeshVertexDataDefinition*)(vertex_data + i * stride)).z;  // 交换X和Z
            vertex_z.y = (*(MeshVertexDataDefinition*)(vertex_data + i * stride)).y;
            vertex_z.z = (*(MeshVertexDataDefinition*)(vertex_data + i * stride)).x;
            vertex_z.u = 2.0f;  // 使用不同UV区分颜色

            vertex_z.nx = vertex_z.ny = vertex_z.nz = 0.0f;
            vertex_z.tx = vertex_z.ty = vertex_z.tz = 0.0f;
            vertex_z.v = 0.0f;
        }

        // 索引缓冲区创建 ===================================================
        size_t index_data_size                        = (4 * segments * 3) * 3 * sizeof(uint16_t);
        m_mesh_data.m_static_mesh_data.m_index_buffer = std::make_shared<BufferData>(index_data_size);
        uint16_t* index_data = static_cast<uint16_t*>(m_mesh_data.m_static_mesh_data.m_index_buffer->m_data);

        // X轴圆柱索引（连接起点和终点圆柱）
        for (int i = 0; i < segments; ++i)
        {
            // 每段形成两个三角形（四边形）
            index_data[0 * segments * 3 + i * 6 + 0] = (uint16_t)(0 * segments + i);
            index_data[0 * segments * 3 + i * 6 + 1] = (uint16_t)1 * segments + i;
            index_data[0 * segments * 3 + i * 6 + 2] = (uint16_t)1 * segments + ((i + 1) % segments);

            index_data[0 * segments * 3 + i * 6 + 3] = (uint16_t)1 * segments + ((i + 1) % segments);
            index_data[0 * segments * 3 + i * 6 + 4] = (uint16_t)0 * segments + ((i + 1) % segments);
            index_data[0 * segments * 3 + i * 6 + 5] = (uint16_t)0 * segments + i;
        }

        // X轴箭头锥体索引
        for (int i = 0; i < segments; ++i)
        {
            // 连接箭头底部中心到锥体顶点
            index_data[2 * segments * 3 + i * 3 + 0] = (uint16_t)3 * segments + 0;                     // 箭头中心
            index_data[2 * segments * 3 + i * 3 + 1] = (uint16_t)2 * segments + i;                     // 锥体顶点
            index_data[2 * segments * 3 + i * 3 + 2] = (uint16_t)2 * segments + ((i + 1) % segments);  // 下一个锥体顶点
        }

        // X轴箭头尖端索引
        for (int i = 0; i < segments; ++i)
        {
            // 连接锥体顶点到箭头尖端
            index_data[3 * segments * 3 + i * 3 + 0] = (uint16_t)2 * segments + i;
            index_data[3 * segments * 3 + i * 3 + 1] = (uint16_t)3 * segments + 1;  // 箭头尖端
            index_data[3 * segments * 3 + i * 3 + 2] = (uint16_t)2 * segments + ((i + 1) % segments);
        }

        // Y轴和Z轴索引（复用X轴索引结构）====================================
        for (int i = 0; i < 4 * segments * 3; ++i)
        {
            // Y轴索引（偏移顶点索引）
            index_data[4 * segments * 3 * 1 + i] = (uint16_t)((3 * segments + 2) * 1) + index_data[i];
            // Z轴索引（偏移顶点索引）
            index_data[4 * segments * 3 * 2 + i] = (uint16_t)((3 * segments + 2) * 2) + index_data[i];
        }
    }

    // 旋转坐标轴构造函数：创建旋转轴几何体
    EditorRotationAxis::EditorRotationAxis()
    {
        // 创建旋转轴渲染网格（环形）
        const float inner_radius = 0.9f;  // 内环半径
        const float outer_radius = 1.0f;  // 外环半径
        const int   segments = 24;        // 环分段数

        uint32_t stride = sizeof(MeshVertexDataDefinition);

        // 顶点缓冲区分配（6组环：XY/YOZ/XOZ的内外环）
        size_t vertex_data_size = 2 * 3 * segments * stride;
        m_mesh_data.m_static_mesh_data.m_vertex_buffer = std::make_shared<BufferData>(vertex_data_size);
        uint8_t* vertex_data = static_cast<uint8_t*>(m_mesh_data.m_static_mesh_data.m_vertex_buffer->m_data);

        // XY平面环 =========================================================
        // 内环
        for (int i = 0; i < segments; i++)
        {
            MeshVertexDataDefinition& vertex = *(MeshVertexDataDefinition*)(vertex_data + (0 * segments + i) * stride);
            vertex.x = cos(2 * Math_PI / segments * i) * inner_radius;
            vertex.y = sin(2 * Math_PI / segments * i) * inner_radius;
            vertex.z = 0.0f;
            vertex.u = 2.0f;  // XY平面标识

            vertex.nx = vertex.ny = vertex.nz = 0.0f;
            vertex.tx = vertex.ty = vertex.tz = 0.0f;
            vertex.v = 0.0f;
        }

        // 外环
        for (int i = 0; i < segments; i++)
        {
            MeshVertexDataDefinition& vertex = *(MeshVertexDataDefinition*)(vertex_data + (1 * segments + i) * stride);
            vertex.x = cos(2 * Math_PI / segments * i) * outer_radius;
            vertex.y = sin(2 * Math_PI / segments * i) * outer_radius;
            vertex.z = 0.0f;
            vertex.u = 2.0f;

            vertex.nx = vertex.ny = vertex.nz = 0.0f;
            vertex.tx = vertex.ty = vertex.tz = 0.0f;
            vertex.v = 0.0f;
        }

        // YOZ平面环 ========================================================
        // 内环
        for (int i = 0; i < segments; i++)
        {
            MeshVertexDataDefinition& vertex = *(MeshVertexDataDefinition*)(vertex_data + (2 * segments + i) * stride);
            vertex.x = 0.0f;
            vertex.y = cos(2 * Math_PI / segments * i) * inner_radius;
            vertex.z = sin(2 * Math_PI / segments * i) * inner_radius;
            vertex.u = 0.0f;  // YOZ平面标识

            vertex.nx = vertex.ny = vertex.nz = 0.0f;
            vertex.tx = vertex.ty = vertex.tz = 0.0f;
            vertex.v = 0.0f;
        }

        // 外环
        for (int i = 0; i < segments; i++)
        {
            MeshVertexDataDefinition& vertex = *(MeshVertexDataDefinition*)(vertex_data + (3 * segments + i) * stride);
            vertex.x = 0.0f;
            vertex.y = cos(2 * Math_PI / segments * i) * outer_radius;
            vertex.z = sin(2 * Math_PI / segments * i) * outer_radius;
            vertex.u = 0.0f;

            vertex.nx = vertex.ny = vertex.nz = 0.0f;
            vertex.tx = vertex.ty = vertex.tz = 0.0f;
            vertex.v = 0.0f;
        }

        // XOZ平面环 ========================================================
        // 内环
        for (int i = 0; i < segments; i++)
        {
            MeshVertexDataDefinition& vertex = *(MeshVertexDataDefinition*)(vertex_data + (4 * segments + i) * stride);
            vertex.x = cos(2 * Math_PI / segments * i) * inner_radius;
            vertex.y = 0.0f;
            vertex.z = sin(2 * Math_PI / segments * i) * inner_radius;
            vertex.u = 1.0f;  // XOZ平面标识

            vertex.nx = vertex.ny = vertex.nz = 0.0f;
            vertex.tx = vertex.ty = vertex.tz = 0.0f;
            vertex.v = 0.0f;
        }

        // 外环
        for (int i = 0; i < segments; i++)
        {
            MeshVertexDataDefinition& vertex = *(MeshVertexDataDefinition*)(vertex_data + (5 * segments + i) * stride);
            vertex.x = cos(2 * Math_PI / segments * i) * outer_radius;
            vertex.y = 0.0f;
            vertex.z = sin(2 * Math_PI / segments * i) * outer_radius;
            vertex.u = 1.0f;

            vertex.nx = vertex.ny = vertex.nz = 0.0f;
            vertex.tx = vertex.ty = vertex.tz = 0.0f;
            vertex.v = 0.0f;
        }

        // 索引缓冲区创建 ===================================================
        size_t index_data_size = 2 * 3 * segments * 3 * sizeof(uint16_t);
        m_mesh_data.m_static_mesh_data.m_index_buffer = std::make_shared<BufferData>(index_data_size);
        uint16_t* index_data = static_cast<uint16_t*>(m_mesh_data.m_static_mesh_data.m_index_buffer->m_data);

        // XY平面索引（形成带状三角形）
        for (int i = 0; i < segments; i++)
        {
            // 内环三角形
            index_data[(3 * i) + 0] = (uint16_t)(i % segments);
            index_data[(3 * i) + 1] = (uint16_t)((i + 1) % segments);
            index_data[(3 * i) + 2] = (uint16_t)(i % segments + segments);

            // 外环三角形
            index_data[1 * 3 * segments + (3 * i) + 0] = (uint16_t)(i % segments + segments);
            index_data[1 * 3 * segments + (3 * i) + 1] = (uint16_t)((i + 1) % segments + segments);
            index_data[1 * 3 * segments + (3 * i) + 2] = (uint16_t)((i + 1) % segments);
        }
        
        // YOZ平面索引（同上）
        for (int i = 0; i < segments; i++)
        {
            index_data[2 * 3 * segments + (3 * i) + 0] = (uint16_t)(i % segments + segments * 2);
            index_data[2 * 3 * segments + (3 * i) + 1] = (uint16_t)((i + 1) % segments + segments * 2);
            index_data[2 * 3 * segments + (3 * i) + 2] = (uint16_t)(i % segments + segments * 3);

            index_data[3 * 3 * segments + (3 * i) + 0] = (uint16_t)(i % segments + segments * 3);
            index_data[3 * 3 * segments + (3 * i) + 1] = (uint16_t)((i + 1) % segments + segments * 3);
            index_data[3 * 3 * segments + (3 * i) + 2] = (uint16_t)((i + 1) % segments + segments * 2);
        }
        
        // XOZ平面索引（同上）
        for (int i = 0; i < segments; i++)
        {
            index_data[4 * 3 * segments + (3 * i) + 0] = (uint16_t)(i % segments + segments * 4);
            index_data[4 * 3 * segments + (3 * i) + 1] = (uint16_t)((i + 1) % segments + segments * 4);
            index_data[4 * 3 * segments + (3 * i) + 2] = (uint16_t)(i % segments + segments * 5);

            index_data[5 * 3 * segments + (3 * i) + 0] = (uint16_t)(i % segments + segments * 5);
            index_data[5 * 3 * segments + (3 * i) + 1] = (uint16_t)((i + 1) % segments + segments * 5);
            index_data[5 * 3 * segments + (3 * i) + 2] = (uint16_t)((i + 1) % segments + segments * 4);
        }
    }

    // 缩放坐标轴构造函数：创建缩放轴几何体
    EditorScaleAxis::EditorScaleAxis()
    {
        const float radius = 0.031f;  // 线半径
        const int   segments = 12;    // 分段数

        uint32_t stride = sizeof(MeshVertexDataDefinition);

        // 顶点缓冲区分配（包含线段和末端方块）
        size_t vertex_data_size = ((2 * segments + 8) * 3 + 8) * stride;
        m_mesh_data.m_static_mesh_data.m_vertex_buffer = std::make_shared<BufferData>(vertex_data_size);
        uint8_t* vertex_data = static_cast<uint8_t*>(m_mesh_data.m_static_mesh_data.m_vertex_buffer->m_data);

        // X轴线段 =========================================================
        // 起点圆柱
        for (int i = 0; i < segments; ++i)
        {
            MeshVertexDataDefinition& vertex = *(MeshVertexDataDefinition*)(vertex_data + (0 * segments + i) * stride);
            vertex.x = 0.0f;
            vertex.y = sin(i * 2 * Math_PI / segments) * radius;
            vertex.z = cos(i * 2 * Math_PI / segments) * radius;
            vertex.u = 0.0f;

            vertex.nx = vertex.ny = vertex.nz = 0.0f;
            vertex.tx = vertex.ty = vertex.tz = 0.0f;
            vertex.v = 0.0f;
        }

        // 终点圆柱
        for (int i = 0; i < segments; ++i)
        {
            MeshVertexDataDefinition& vertex = *(MeshVertexDataDefinition*)(vertex_data + (1 * segments + i) * stride);
            vertex.x = 1.6 - radius * 10;  // 线段长度
            vertex.y = (*(MeshVertexDataDefinition*)(vertex_data + (0 * segments + i) * stride)).y;
            vertex.z = (*(MeshVertexDataDefinition*)(vertex_data + (0 * segments + i) * stride)).z;
            vertex.u = 0.0f;

            vertex.nx = vertex.ny = vertex.nz = 0.0f;
            vertex.tx = vertex.ty = vertex.tz = 0.0f;
            vertex.v = 0.0f;
        }

        // X轴末端方块（8个顶点）
        {
            // 方块底部四个点
            MeshVertexDataDefinition& vertex0 = *(MeshVertexDataDefinition*)(vertex_data + (2 * segments + 0) * stride);
            vertex0.x = 1.6 - radius * 10;
            vertex0.y = +radius * 5;
            vertex0.z = +radius * 5;
            vertex0.u = 0.0f;

            vertex0.nx = vertex0.ny = vertex0.nz = 0.0f;
            vertex0.tx = vertex0.ty = vertex0.tz = 0.0f;
            vertex0.v = 0.0f;

            MeshVertexDataDefinition& vertex1 = *(MeshVertexDataDefinition*)(vertex_data + (2 * segments + 1) * stride);
            vertex1.x = 1.6 - radius * 10;
            vertex1.y = +radius * 5;
            vertex1.z = -radius * 5;
            vertex1.u = 0.0f;

            vertex1.nx = vertex1.ny = vertex1.nz = 0.0f;
            vertex1.tx = vertex1.ty = vertex1.tz = 0.0f;
            vertex1.v = 0.0f;

            MeshVertexDataDefinition& vertex2 = *(MeshVertexDataDefinition*)(vertex_data + (2 * segments + 2) * stride);
            vertex2.x = 1.6 - radius * 10;
            vertex2.y = -radius * 5;
            vertex2.z = +radius * 5;
            vertex2.u = 0.0f;

            vertex2.nx = vertex2.ny = vertex2.nz = 0.0f;
            vertex2.tx = vertex2.ty = vertex2.tz = 0.0f;
            vertex2.v = 0.0f;

            MeshVertexDataDefinition& vertex3 = *(MeshVertexDataDefinition*)(vertex_data + (2 * segments + 3) * stride);
            vertex3.x = 1.6 - radius * 10;
            vertex3.y = -radius * 5;
            vertex3.z = -radius * 5;
            vertex3.u = 0.0f;

            vertex3.nx = vertex3.ny = vertex3.nz = 0.0f;
            vertex3.tx = vertex3.ty = vertex3.tz = 0.0f;
            vertex3.v = 0.0f;

            MeshVertexDataDefinition& vertex4 = *(MeshVertexDataDefinition*)(vertex_data + (2 * segments + 4) * stride);
            vertex4.x = 1.6;
            vertex4.y = +radius * 5;
            vertex4.z = +radius * 5;
            vertex4.u = 0.0f;

            vertex4.nx = vertex4.ny = vertex4.nz = 0.0f;
            vertex4.tx = vertex4.ty = vertex4.tz = 0.0f;
            vertex4.v = 0.0f;

            MeshVertexDataDefinition& vertex5 = *(MeshVertexDataDefinition*)(vertex_data + (2 * segments + 5) * stride);
            vertex5.x = 1.6;
            vertex5.y = +radius * 5;
            vertex5.z = -radius * 5;
            vertex5.u = 0.0f;

            vertex5.nx = vertex5.ny = vertex5.nz = 0.0f;
            vertex5.tx = vertex5.ty = vertex5.tz = 0.0f;
            vertex5.v = 0.0f;

            MeshVertexDataDefinition& vertex6 = *(MeshVertexDataDefinition*)(vertex_data + (2 * segments + 6) * stride);
            vertex6.x = 1.6;
            vertex6.y = -radius * 5;
            vertex6.z = +radius * 5;
            vertex6.u = 0.0f;

            vertex6.nx = vertex6.ny = vertex6.nz = 0.0f;
            vertex6.tx = vertex6.ty = vertex6.tz = 0.0f;
            vertex6.v = 0.0f;

            MeshVertexDataDefinition& vertex7 = *(MeshVertexDataDefinition*)(vertex_data + (2 * segments + 7) * stride);
            vertex7.x = 1.6;
            vertex7.y = -radius * 5;
            vertex7.z = -radius * 5;
            vertex7.u = 0.0f;

            vertex7.nx = vertex7.ny = vertex7.nz = 0.0f;
            vertex7.tx = vertex7.ty = vertex7.tz = 0.0f;
            vertex7.v = 0.0f;
        }

        // Y轴和Z轴构造（旋转X轴几何）=======================================
        for (int i = 0; i < 2 * segments + 8; ++i)
        {
            MeshVertexDataDefinition& vertex1 = *(MeshVertexDataDefinition*)(vertex_data + ((2 * segments + 8) * 1 + i) * stride);
            vertex1.x = -(*(MeshVertexDataDefinition*)(vertex_data + i * stride)).y; // 坐标变换
            vertex1.y = (*(MeshVertexDataDefinition*)(vertex_data + i * stride)).x;
            vertex1.z = (*(MeshVertexDataDefinition*)(vertex_data + i * stride)).z;
            vertex1.u = 1.0f;

            vertex1.nx = vertex1.ny = vertex1.nz = 0.0f;
            vertex1.tx = vertex1.ty = vertex1.tz = 0.0f;
            vertex1.v = 0.0f;

            MeshVertexDataDefinition& vertex2 = *(MeshVertexDataDefinition*)(vertex_data + ((2 * segments + 8) * 2 + i) * stride);
            vertex2.x = -(*(MeshVertexDataDefinition*)(vertex_data + i * stride)).z;
            vertex2.y = (*(MeshVertexDataDefinition*)(vertex_data + i * stride)).y;
            vertex2.z = (*(MeshVertexDataDefinition*)(vertex_data + i * stride)).x;
            vertex2.u = 2.0f;

            vertex2.nx = vertex2.ny = vertex2.nz = 0.0f;
            vertex2.tx = vertex2.ty = vertex2.tz = 0.0f;
            vertex2.v = 0.0f;
        }

        int start_vertex_index = (2 * segments + 8) * 3;
        {
            MeshVertexDataDefinition& vertex0 =
                *(MeshVertexDataDefinition*)(vertex_data + (start_vertex_index + 0) * stride);
            vertex0.x = 0.0f;
            vertex0.y = 0.0f;
            vertex0.z = 0.0f;
            vertex0.u = 6.0f;

            vertex0.nx = vertex0.ny = vertex0.nz = 0.0f;
            vertex0.tx = vertex0.ty = vertex0.tz = 0.0f;
            vertex0.v = 0.0f;

            MeshVertexDataDefinition& vertex1 =
                *(MeshVertexDataDefinition*)(vertex_data + (start_vertex_index + 1) * stride);
            vertex1.x = 0.1f;
            vertex1.y = 0.0f;
            vertex1.z = 0.0f;
            vertex1.u = 6.0f;

            vertex1.nx = vertex1.ny = vertex1.nz = 0.0f;
            vertex1.tx = vertex1.ty = vertex1.tz = 0.0f;
            vertex1.v = 0.0f;

            MeshVertexDataDefinition& vertex2 =
                *(MeshVertexDataDefinition*)(vertex_data + (start_vertex_index + 2) * stride);
            vertex2.x = 0.1f;
            vertex2.y = 0.1f;
            vertex2.z = 0.0f;
            vertex2.u = 6.0f;

            vertex2.nx = vertex2.ny = vertex2.nz = 0.0f;
            vertex2.tx = vertex2.ty = vertex2.tz = 0.0f;
            vertex2.v = 0.0f;

            MeshVertexDataDefinition& vertex3 =
                *(MeshVertexDataDefinition*)(vertex_data + (start_vertex_index + 3) * stride);
            vertex3.x = 0.0f;
            vertex3.y = 0.1f;
            vertex3.z = 0.0f;
            vertex3.u = 6.0f;

            vertex3.nx = vertex3.ny = vertex3.nz = 0.0f;
            vertex3.tx = vertex3.ty = vertex3.tz = 0.0f;
            vertex3.v = 0.0f;

            MeshVertexDataDefinition& vertex4 =
                *(MeshVertexDataDefinition*)(vertex_data + (start_vertex_index + 4) * stride);
            vertex4.x = 0.0f;
            vertex4.y = 0.0f;
            vertex4.z = 0.1f;
            vertex4.u = 6.0f;

            vertex4.nx = vertex4.ny = vertex4.nz = 0.0f;
            vertex4.tx = vertex4.ty = vertex4.tz = 0.0f;
            vertex4.v = 0.0f;

            MeshVertexDataDefinition& vertex5 =
                *(MeshVertexDataDefinition*)(vertex_data + (start_vertex_index + 5) * stride);
            vertex5.x = 0.1f;
            vertex5.y = 0.0f;
            vertex5.z = 0.1f;
            vertex5.u = 6.0f;

            vertex5.nx = vertex5.ny = vertex5.nz = 0.0f;
            vertex5.tx = vertex5.ty = vertex5.tz = 0.0f;
            vertex5.v = 0.0f;

            MeshVertexDataDefinition& vertex6 =
                *(MeshVertexDataDefinition*)(vertex_data + (start_vertex_index + 6) * stride);
            vertex6.x = 0.1f;
            vertex6.y = 0.1f;
            vertex6.z = 0.1f;
            vertex6.u = 6.0f;

            vertex6.nx = vertex6.ny = vertex6.nz = 0.0f;
            vertex6.tx = vertex6.ty = vertex6.tz = 0.0f;
            vertex6.v = 0.0f;

            MeshVertexDataDefinition& vertex7 =
                *(MeshVertexDataDefinition*)(vertex_data + (start_vertex_index + 7) * stride);
            vertex7.x = 0.0f;
            vertex7.y = 0.1f;
            vertex7.z = 0.1f;
            vertex7.u = 6.0f;

            vertex7.nx = vertex7.ny = vertex7.nz = 0.0f;
            vertex7.tx = vertex7.ty = vertex7.tz = 0.0f;
            vertex7.v = 0.0f;
        }

        // index
        size_t index_data_size = (((2 * segments + 12) * 3) * 3 + 3 * 2 * 6) * sizeof(uint16_t);
        m_mesh_data.m_static_mesh_data.m_index_buffer = std::make_shared<BufferData>(index_data_size);
        uint16_t* index_data = static_cast<uint16_t*>(m_mesh_data.m_static_mesh_data.m_index_buffer->m_data);

        for (int i = 0; i < segments; ++i)
        {
            index_data[0 * segments * 3 + i * 6 + 0] = (uint16_t)(0 * segments + i);
            index_data[0 * segments * 3 + i * 6 + 1] = (uint16_t)(1 * segments + i);
            index_data[0 * segments * 3 + i * 6 + 2] = (uint16_t)(1 * segments + ((i + 1) % segments));

            index_data[0 * segments * 3 + i * 6 + 3] = (uint16_t)(1 * segments + ((i + 1) % segments));
            index_data[0 * segments * 3 + i * 6 + 4] = (uint16_t)(0 * segments + ((i + 1) % segments));
            index_data[0 * segments * 3 + i * 6 + 5] = (uint16_t)(0 * segments + i);
        }
        {
            index_data[2 * segments * 3 + 0 * 3 + 0] = (uint16_t)(2 * segments + 0);
            index_data[2 * segments * 3 + 0 * 3 + 1] = (uint16_t)(2 * segments + 1);
            index_data[2 * segments * 3 + 0 * 3 + 2] = (uint16_t)(2 * segments + 3);
            index_data[2 * segments * 3 + 1 * 3 + 0] = (uint16_t)(2 * segments + 3);
            index_data[2 * segments * 3 + 1 * 3 + 1] = (uint16_t)(2 * segments + 2);
            index_data[2 * segments * 3 + 1 * 3 + 2] = (uint16_t)(2 * segments + 0);

            index_data[2 * segments * 3 + 2 * 3 + 0] = (uint16_t)(2 * segments + 1);
            index_data[2 * segments * 3 + 2 * 3 + 1] = (uint16_t)(2 * segments + 5);
            index_data[2 * segments * 3 + 2 * 3 + 2] = (uint16_t)(2 * segments + 7);
            index_data[2 * segments * 3 + 3 * 3 + 0] = (uint16_t)(2 * segments + 7);
            index_data[2 * segments * 3 + 3 * 3 + 1] = (uint16_t)(2 * segments + 3);
            index_data[2 * segments * 3 + 3 * 3 + 2] = (uint16_t)(2 * segments + 1);

            index_data[2 * segments * 3 + 4 * 3 + 0] = (uint16_t)(2 * segments + 5);
            index_data[2 * segments * 3 + 4 * 3 + 1] = (uint16_t)(2 * segments + 4);
            index_data[2 * segments * 3 + 4 * 3 + 2] = (uint16_t)(2 * segments + 6);
            index_data[2 * segments * 3 + 5 * 3 + 0] = (uint16_t)(2 * segments + 6);
            index_data[2 * segments * 3 + 5 * 3 + 1] = (uint16_t)(2 * segments + 7);
            index_data[2 * segments * 3 + 5 * 3 + 2] = (uint16_t)(2 * segments + 5);

            index_data[2 * segments * 3 + 6 * 3 + 0] = (uint16_t)(2 * segments + 4);
            index_data[2 * segments * 3 + 6 * 3 + 1] = (uint16_t)(2 * segments + 0);
            index_data[2 * segments * 3 + 6 * 3 + 2] = (uint16_t)(2 * segments + 2);
            index_data[2 * segments * 3 + 7 * 3 + 0] = (uint16_t)(2 * segments + 2);
            index_data[2 * segments * 3 + 7 * 3 + 1] = (uint16_t)(2 * segments + 6);
            index_data[2 * segments * 3 + 7 * 3 + 2] = (uint16_t)(2 * segments + 4);

            index_data[2 * segments * 3 + 8 * 3 + 0] = (uint16_t)(2 * segments + 4);
            index_data[2 * segments * 3 + 8 * 3 + 1] = (uint16_t)(2 * segments + 5);
            index_data[2 * segments * 3 + 8 * 3 + 2] = (uint16_t)(2 * segments + 1);
            index_data[2 * segments * 3 + 9 * 3 + 0] = (uint16_t)(2 * segments + 1);
            index_data[2 * segments * 3 + 9 * 3 + 1] = (uint16_t)(2 * segments + 0);
            index_data[2 * segments * 3 + 9 * 3 + 2] = (uint16_t)(2 * segments + 4);

            index_data[2 * segments * 3 + 10 * 3 + 0] = (uint16_t)(2 * segments + 2);
            index_data[2 * segments * 3 + 10 * 3 + 1] = (uint16_t)(2 * segments + 3);
            index_data[2 * segments * 3 + 10 * 3 + 2] = (uint16_t)(2 * segments + 7);
            index_data[2 * segments * 3 + 11 * 3 + 0] = (uint16_t)(2 * segments + 7);
            index_data[2 * segments * 3 + 11 * 3 + 1] = (uint16_t)(2 * segments + 6);
            index_data[2 * segments * 3 + 11 * 3 + 2] = (uint16_t)(2 * segments + 2);
        }

        for (uint16_t i = 0; i < (2 * segments + 12) * 3; ++i)
        {
            index_data[(2 * segments + 12) * 3 * 1 + i] = (uint16_t)((2 * segments + 8) * 1 + index_data[i]);
            index_data[(2 * segments + 12) * 3 * 2 + i] = (uint16_t)((2 * segments + 8) * 2 + index_data[i]);
        }

        int start_index = ((2 * segments + 12) * 3) * 3;
        index_data[start_index + 0 * 3 + 0] = (uint16_t)(start_vertex_index + 0);
        index_data[start_index + 0 * 3 + 1] = (uint16_t)(start_vertex_index + 1);
        index_data[start_index + 0 * 3 + 2] = (uint16_t)(start_vertex_index + 2);
        index_data[start_index + 1 * 3 + 0] = (uint16_t)(start_vertex_index + 0);
        index_data[start_index + 1 * 3 + 1] = (uint16_t)(start_vertex_index + 2);
        index_data[start_index + 1 * 3 + 2] = (uint16_t)(start_vertex_index + 3);

        index_data[start_index + 2 * 3 + 0] = (uint16_t)(start_vertex_index + 4);
        index_data[start_index + 2 * 3 + 1] = (uint16_t)(start_vertex_index + 5);
        index_data[start_index + 2 * 3 + 2] = (uint16_t)(start_vertex_index + 6);
        index_data[start_index + 3 * 3 + 0] = (uint16_t)(start_vertex_index + 4);
        index_data[start_index + 3 * 3 + 1] = (uint16_t)(start_vertex_index + 6);
        index_data[start_index + 3 * 3 + 2] = (uint16_t)(start_vertex_index + 7);

        index_data[start_index + 4 * 3 + 0] = (uint16_t)(start_vertex_index + 0);
        index_data[start_index + 4 * 3 + 1] = (uint16_t)(start_vertex_index + 1);
        index_data[start_index + 4 * 3 + 2] = (uint16_t)(start_vertex_index + 5);
        index_data[start_index + 5 * 3 + 0] = (uint16_t)(start_vertex_index + 0);
        index_data[start_index + 5 * 3 + 1] = (uint16_t)(start_vertex_index + 5);
        index_data[start_index + 5 * 3 + 2] = (uint16_t)(start_vertex_index + 4);

        index_data[start_index + 6 * 3 + 0] = (uint16_t)(start_vertex_index + 3);
        index_data[start_index + 6 * 3 + 1] = (uint16_t)(start_vertex_index + 2);
        index_data[start_index + 6 * 3 + 2] = (uint16_t)(start_vertex_index + 6);
        index_data[start_index + 7 * 3 + 0] = (uint16_t)(start_vertex_index + 3);
        index_data[start_index + 7 * 3 + 1] = (uint16_t)(start_vertex_index + 6);
        index_data[start_index + 7 * 3 + 2] = (uint16_t)(start_vertex_index + 7);

        index_data[start_index + 8 * 3 + 0] = (uint16_t)(start_vertex_index + 4);
        index_data[start_index + 8 * 3 + 1] = (uint16_t)(start_vertex_index + 5);
        index_data[start_index + 8 * 3 + 2] = (uint16_t)(start_vertex_index + 1);
        index_data[start_index + 9 * 3 + 0] = (uint16_t)(start_vertex_index + 1);
        index_data[start_index + 9 * 3 + 1] = (uint16_t)(start_vertex_index + 0);
        index_data[start_index + 9 * 3 + 2] = (uint16_t)(start_vertex_index + 4);

        index_data[start_index + 10 * 3 + 0] = (uint16_t)(start_vertex_index + 2);
        index_data[start_index + 10 * 3 + 1] = (uint16_t)(start_vertex_index + 3);
        index_data[start_index + 10 * 3 + 2] = (uint16_t)(start_vertex_index + 7);
        index_data[start_index + 11 * 3 + 0] = (uint16_t)(start_vertex_index + 7);
        index_data[start_index + 11 * 3 + 1] = (uint16_t)(start_vertex_index + 6);
        index_data[start_index + 11 * 3 + 2] = (uint16_t)(start_vertex_index + 2);
    }
}