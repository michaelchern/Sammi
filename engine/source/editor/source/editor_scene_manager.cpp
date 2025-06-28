#include <cassert>
#include <mutex>  // 互斥锁支持

#include "editor/include/editor.h"                 // 编辑器主类
#include "editor/include/editor_global_context.h"  // 全局上下文
#include "editor/include/editor_scene_manager.h"   // 场景管理器

#include "runtime/core/base/macro.h"  // 宏定义

#include "runtime/engine.h"                                                      // 引擎核心
#include "runtime/function/framework/component/transform/transform_component.h"  // 变换组件
#include "runtime/function/framework/level/level.h"                              // 关卡管理
#include "runtime/function/framework/world/world_manager.h"                      // 世界管理
#include "runtime/function/input/input_system.h"                                 // 输入系统
#include "runtime/function/render/render_camera.h"                               // 渲染相机
#include "runtime/function/render/render_system.h"                               // 渲染系统

namespace Sammi
{
    /// 初始化场景管理器
    void EditorSceneManager::initialize() {}

    /// 每帧更新场景状态
    void EditorSceneManager::tick(float delta_time)
    {
        // 获取选中的游戏对象
        std::shared_ptr<GObject> selected_gobject = getSelectedGObject().lock();
        if (selected_gobject)
        {
            // 获取变换组件并标记为脏（需要更新）
            TransformComponent* transform_component = selected_gobject->tryGetComponent(TransformComponent);
            if (transform_component)
            {
                transform_component->setDirtyFlag(true);
            }
        }
    }

    /// 计算射线与平面的交点
    float intersectPlaneRay(Vector3 normal, float d, Vector3 origin, Vector3 dir)
    {
        float deno = normal.dotProduct(dir);
        if (fabs(deno) < 0.0001)  // 避免除零错误
        {
            deno = 0.0001;
        }

        return -(normal.dotProduct(origin) + d) / deno;
    }

    /// 更新光标在坐标轴上的状态
    size_t EditorSceneManager::updateCursorOnAxis(Vector2 cursor_uv, Vector2 game_engine_window_size)
    {
        // 获取相机参数
        float   camera_fov      = m_camera->getFovYDeprecated();
        Vector3 camera_forward  = m_camera->forward();
        Vector3 camera_up       = m_camera->up();
        Vector3 camera_right    = m_camera->right();
        Vector3 camera_position = m_camera->position();

        // 如果没有选中对象，返回当前轴状态
        if (m_selected_gobject_id == k_invalid_gobject_id)
        {
            return m_selected_axis;
        }

        // 获取当前坐标轴类型的网格
        RenderEntity* selected_aixs = getAxisMeshByType(m_axis_mode);
        m_selected_axis             = 3;  // 默认无选中

        // 如果不显示坐标轴，直接返回
        if (m_is_show_axis == false)
        {
            return m_selected_axis;
        }
        else
        {
            // 获取模型矩阵并分解
            Matrix4x4 model_matrix = selected_aixs->m_model_matrix;
            Vector3 model_scale;
            Quaternion model_rotation;
            Vector3 model_translation;
            model_matrix.decomposition(model_translation, model_scale, model_rotation);

            // 计算屏幕中心UV坐标
            float   window_forward   = game_engine_window_size.y / 2.0f / Math::tan(Math::degreesToRadians(camera_fov) / 2.0f);
            Vector2 screen_center_uv = Vector2(cursor_uv.x, 1 - cursor_uv.y) - Vector2(0.5, 0.5);

            // 计算世界空间中的射线方向
            Vector3 world_ray_dir = camera_forward * window_forward +
                                    camera_right * (float)game_engine_window_size.x * screen_center_uv.x +
                                    camera_up * (float)game_engine_window_size.y * screen_center_uv.y;

            // 将射线转换到模型空间
            Vector4 local_ray_origin     = model_matrix.inverse() * Vector4(camera_position, 1.0f);
            Vector3 local_ray_origin_xyz = Vector3(local_ray_origin.x, local_ray_origin.y, local_ray_origin.z);
            Quaternion inversed_rotation = model_rotation.inverse();
            inversed_rotation.normalise();
            Vector3 local_ray_dir        = inversed_rotation * world_ray_dir;

            // 定义三个坐标平面
            Vector3 plane_normals[3] = { Vector3(1, 0, 0), Vector3(0, 1, 0), Vector3(0, 0, 1)};

            // 计算射线与各平面的交点
            float plane_view_depth[3] = {intersectPlaneRay(plane_normals[0], 0, local_ray_origin_xyz, local_ray_dir),
                                         intersectPlaneRay(plane_normals[1], 0, local_ray_origin_xyz, local_ray_dir),
                                         intersectPlaneRay(plane_normals[2], 0, local_ray_origin_xyz, local_ray_dir)};

            Vector3 intersect_pt[3] = {local_ray_origin_xyz + plane_view_depth[0] * local_ray_dir,   // YOZ平面
                                       local_ray_origin_xyz + plane_view_depth[1] * local_ray_dir,   // XOZ平面
                                       local_ray_origin_xyz + plane_view_depth[2] * local_ray_dir};  // XOY平面

            // 根据坐标轴模式处理
            if ((int)m_axis_mode == 0 || (int)m_axis_mode == 2)
            {
                // 平移和缩放模式处理逻辑
                const float DIST_THRESHOLD   = 0.6f;
                const float EDGE_OF_AXIS_MIN = 0.1f;
                const float EDGE_OF_AXIS_MAX = 2.0f;
                const float AXIS_LENGTH      = 2.0f;

                float max_dist = 0.0f;
                // 检查射线是否在平面上
                for (int i = 0; i < 3; ++i)
                {
                    // 计算射线与平面法线的夹角
                    float local_ray_dir_proj = Math::abs(local_ray_dir.dotProduct(plane_normals[i]));
                    float cos_alpha          = local_ray_dir_proj / 1.0f;
                    if (cos_alpha <= 0.15)  // 约80-100度范围
                    {
                        // 计算相邻轴索引
                        int   index00   = (i + 1) % 3;
                        int   index01   = 3 - i - index00;
                        int   index10   = (i + 2) % 3;
                        int   index11   = 3 - i - index10;

                        // 检查距离阈值
                        float axis_dist = (Math::abs(intersect_pt[index00][i]) + Math::abs(intersect_pt[index10][i])) / 2;
                        if (axis_dist > DIST_THRESHOLD)
                        {
                            continue;
                        }

                        // 检查并选择最近的轴
                        if ((intersect_pt[index00][index01] > EDGE_OF_AXIS_MIN) &&
                            (intersect_pt[index00][index01] < AXIS_LENGTH) &&
                            (intersect_pt[index00][index01] > max_dist) &&
                            (Math::abs(intersect_pt[index00][i]) < EDGE_OF_AXIS_MAX))
                        {
                            max_dist        = intersect_pt[index00][index01];
                            m_selected_axis = index01;
                        }
                        if ((intersect_pt[index10][index11] > EDGE_OF_AXIS_MIN) &&
                            (intersect_pt[index10][index11] < AXIS_LENGTH) &&
                            (intersect_pt[index10][index11] > max_dist) &&
                            (Math::abs(intersect_pt[index10][i]) < EDGE_OF_AXIS_MAX))
                        {
                            max_dist        = intersect_pt[index10][index11];
                            m_selected_axis = index11;
                        }
                    }
                }

                // 如果没有选中轴，检查交点
                if (m_selected_axis == 3)
                {
                    float min_dist = 1e10f;
                    for (int i = 0; i < 3; ++i)
                    {
                        int   index0 = (i + 1) % 3;
                        int   index1 = (i + 2) % 3;
                        float dist = Math::sqr(intersect_pt[index0][index1]) + Math::sqr(intersect_pt[index1][index0]);
                        if ((intersect_pt[index0][i] > EDGE_OF_AXIS_MIN) &&
                            (intersect_pt[index0][i] < EDGE_OF_AXIS_MAX) &&
                            (dist < DIST_THRESHOLD) &&
                            (dist < min_dist))
                        {
                            min_dist        = dist;
                            m_selected_axis = i;
                        }
                    }
                }
            }
            else if ((int)m_axis_mode == 1) // 旋转模式
            {
                const float DIST_THRESHOLD = 0.2f;
                float min_dist = 1e10f;
                for (int i = 0; i < 3; ++i)
                {
                    // 计算点到旋转环的距离
                    const float dist = std::fabs(1 - std::hypot(intersect_pt[i].x, intersect_pt[i].y, intersect_pt[i].z));
                    if ((dist < DIST_THRESHOLD) && (dist < min_dist))
                    {
                        min_dist        = dist;
                        m_selected_axis = i;
                    }
                }
            }
            else
            {
                return m_selected_axis;
            }
        }

        // 通知渲染系统选中的轴
        g_editor_global_context.m_render_system->setSelectedAxis(m_selected_axis);
        return m_selected_axis;
    }

    /// 根据坐标轴模式获取对应的网格
    RenderEntity* EditorSceneManager::getAxisMeshByType(EditorAxisMode axis_mode)
    {
        RenderEntity* axis_mesh = nullptr;
        switch (axis_mode)
        {
            case EditorAxisMode::TranslateMode:
                axis_mesh = &m_translation_axis;
                break;
            case EditorAxisMode::RotateMode:
                axis_mesh = &m_rotation_axis;
                break;
            case EditorAxisMode::ScaleMode:
                axis_mesh = &m_scale_aixs;
                break;
            default:
                break;
        }
        return axis_mesh;
    }

    /// 绘制选中实体的坐标轴
    void EditorSceneManager::drawSelectedEntityAxis()
    {
        // 获取选中的游戏对象
        std::shared_ptr<GObject> selected_object = getSelectedGObject().lock();

        if (g_is_editor_mode && selected_object != nullptr)
        {
            // 获取变换组件
            const TransformComponent* transform_component = selected_object->tryGetComponentConst(TransformComponent);

            // 分解变换矩阵
            Vector3    scale;
            Quaternion rotation;
            Vector3    translation;
            transform_component->getMatrix().decomposition(translation, scale, rotation);

            // 构建坐标轴模型矩阵
            Matrix4x4 translation_matrix = Matrix4x4::getTrans(translation);
            Matrix4x4 scale_matrix       = Matrix4x4::buildScaleMatrix(1.0f, 1.0f, 1.0f);
            Matrix4x4 axis_model_matrix  = translation_matrix * scale_matrix;

            // 获取当前坐标轴类型的渲染实体
            RenderEntity* selected_aixs = getAxisMeshByType(m_axis_mode);

            // 根据模式设置模型矩阵
            if (m_axis_mode == EditorAxisMode::TranslateMode || m_axis_mode == EditorAxisMode::RotateMode)
            {
                selected_aixs->m_model_matrix = axis_model_matrix;
            }
            else if (m_axis_mode == EditorAxisMode::ScaleMode)
            {
                selected_aixs->m_model_matrix = axis_model_matrix * Matrix4x4(rotation);
            }

            // 通知渲染系统显示坐标轴
            g_editor_global_context.m_render_system->setVisibleAxis(*selected_aixs);
        }
        else
        {
            // 隐藏坐标轴
            g_editor_global_context.m_render_system->setVisibleAxis(std::nullopt);
        }
    }

    /// 获取选中的游戏对象
    std::weak_ptr<GObject> EditorSceneManager::getSelectedGObject() const
    {
        std::weak_ptr<GObject> selected_object;
        if (m_selected_gobject_id != k_invalid_gobject_id)
        {
            // 从当前关卡获取游戏对象
            std::shared_ptr<Level> level = g_runtime_global_context.m_world_manager->getCurrentActiveLevel().lock();
            if (level != nullptr)
            {
                selected_object = level->getGObjectByID(m_selected_gobject_id);
            }
        }
        return selected_object;
    }

    /// 当游戏对象被选中时调用
    void EditorSceneManager::onGObjectSelected(GObjectID selected_gobject_id)
    {
        if (selected_gobject_id == m_selected_gobject_id)
            return;

        m_selected_gobject_id = selected_gobject_id;

        // 获取并保存选中对象的变换矩阵
        std::shared_ptr<GObject> selected_gobject = getSelectedGObject().lock();
        if (selected_gobject)
        {
            const TransformComponent* transform_component = selected_gobject->tryGetComponentConst(TransformComponent);
            m_selected_object_matrix                      = transform_component->getMatrix();
        }

        // 绘制坐标轴并记录日志
        drawSelectedEntityAxis();

        if (m_selected_gobject_id != k_invalid_gobject_id)
        {
            LOG_INFO("select game object " + std::to_string(m_selected_gobject_id));
        }
        else
        {
            LOG_INFO("no game object selected");
        }
    }

    /// 删除选中的游戏对象
    void EditorSceneManager::onDeleteSelectedGObject()
    {
        std::shared_ptr<GObject> selected_object = getSelectedGObject().lock();
        if (selected_object != nullptr)
        {
            // 从当前关卡删除对象
            std::shared_ptr<Level> current_active_level = g_runtime_global_context.m_world_manager->getCurrentActiveLevel().lock();
            if (current_active_level == nullptr)
                return;

            current_active_level->deleteGObjectByID(m_selected_gobject_id);

            // 通知渲染系统删除对象
            RenderSwapContext& swap_context = g_editor_global_context.m_render_system->getSwapContext();
            swap_context.getLogicSwapData().addDeleteGameObject(GameObjectDesc {selected_object->getID(), {}});
        }

        // 清除选中状态
        onGObjectSelected(k_invalid_gobject_id);
    }

    /// 移动实体（核心变换逻辑）
    void EditorSceneManager::moveEntity(float     new_mouse_pos_x,
                                        float     new_mouse_pos_y,
                                        float     last_mouse_pos_x,
                                        float     last_mouse_pos_y,
                                        Vector2   engine_window_pos,
                                        Vector2   engine_window_size,
                                        size_t    cursor_on_axis,
                                        Matrix4x4 model_matrix)
    {
        // 获取选中的游戏对象
        std::shared_ptr<GObject> selected_object = getSelectedGObject().lock();
        if (selected_object == nullptr)
            return;

        // 计算角度速度（基于窗口大小）
        float   angularVelocity     = 18.0f / Math::max(engine_window_size.x, engine_window_size.y);
        Vector2 delta_mouse_move_uv = {(new_mouse_pos_x - last_mouse_pos_x), (new_mouse_pos_y - last_mouse_pos_y)};

        // 分解模型矩阵
        Vector3    model_scale;
        Quaternion model_rotation;
        Vector3    model_translation;
        model_matrix.decomposition(model_translation, model_scale, model_rotation);

        // 构建坐标轴模型矩阵
        Matrix4x4 axis_model_matrix = Matrix4x4::IDENTITY;
        axis_model_matrix.setTrans(model_translation);

        // 获取视图和投影矩阵
        Matrix4x4 view_matrix = m_camera->getLookAtMatrix();
        Matrix4x4 proj_matrix = m_camera->getPersProjMatrix();

        // 计算模型在世界和裁剪空间的位置
        Vector4 model_world_position_4(model_translation, 1.f);
        Vector4 model_origin_clip_position = proj_matrix * view_matrix * model_world_position_4;
        model_origin_clip_position /= model_origin_clip_position.w;
        Vector2 model_origin_clip_uv = Vector2((model_origin_clip_position.x + 1) / 2.0f, (model_origin_clip_position.y + 1) / 2.0f);

        // 计算各坐标轴在裁剪空间的方向
        Vector4 axis_x_local_position_4(1, 0, 0, 1);
        if (m_axis_mode == EditorAxisMode::ScaleMode)
        {
            axis_x_local_position_4 = Matrix4x4(model_rotation) * axis_x_local_position_4;
        }
        Vector4 axis_x_world_position_4 = axis_model_matrix * axis_x_local_position_4;
        axis_x_world_position_4.w       = 1.0f;
        Vector4 axis_x_clip_position    = proj_matrix * view_matrix * axis_x_world_position_4;
        axis_x_clip_position /= axis_x_clip_position.w;
        Vector2 axis_x_clip_uv((axis_x_clip_position.x + 1) / 2.0f, (axis_x_clip_position.y + 1) / 2.0f);
        Vector2 axis_x_direction_uv = axis_x_clip_uv - model_origin_clip_uv;
        axis_x_direction_uv.normalise();

        Vector4 axis_y_local_position_4(0, 1, 0, 1);
        if (m_axis_mode == EditorAxisMode::ScaleMode)
        {
            axis_y_local_position_4 = Matrix4x4(model_rotation) * axis_y_local_position_4;
        }
        Vector4 axis_y_world_position_4 = axis_model_matrix * axis_y_local_position_4;
        axis_y_world_position_4.w       = 1.0f;
        Vector4 axis_y_clip_position    = proj_matrix * view_matrix * axis_y_world_position_4;
        axis_y_clip_position /= axis_y_clip_position.w;
        Vector2 axis_y_clip_uv((axis_y_clip_position.x + 1) / 2.0f, (axis_y_clip_position.y + 1) / 2.0f);
        Vector2 axis_y_direction_uv = axis_y_clip_uv - model_origin_clip_uv;
        axis_y_direction_uv.normalise();

        Vector4 axis_z_local_position_4(0, 0, 1, 1);
        if (m_axis_mode == EditorAxisMode::ScaleMode)
        {
            axis_z_local_position_4 = Matrix4x4(model_rotation) * axis_z_local_position_4;
        }
        Vector4 axis_z_world_position_4 = axis_model_matrix * axis_z_local_position_4;
        axis_z_world_position_4.w       = 1.0f;
        Vector4 axis_z_clip_position    = proj_matrix * view_matrix * axis_z_world_position_4;
        axis_z_clip_position /= axis_z_clip_position.w;
        Vector2 axis_z_clip_uv((axis_z_clip_position.x + 1) / 2.0f, (axis_z_clip_position.y + 1) / 2.0f);
        Vector2 axis_z_direction_uv = axis_z_clip_uv - model_origin_clip_uv;
        axis_z_direction_uv.normalise();

        // 获取变换组件
        TransformComponent* transform_component = selected_object->tryGetComponent(TransformComponent);
        Matrix4x4 new_model_matrix(Matrix4x4::IDENTITY);

        // 根据坐标轴模式执行不同的变换
        if (m_axis_mode == EditorAxisMode::TranslateMode)
        {
            Vector3 move_vector = {0, 0, 0};
            if (cursor_on_axis == 0)
            {
                move_vector.x = delta_mouse_move_uv.dotProduct(axis_x_direction_uv) * angularVelocity;
            }
            else if (cursor_on_axis == 1)
            {
                move_vector.y = delta_mouse_move_uv.dotProduct(axis_y_direction_uv) * angularVelocity;
            }
            else if (cursor_on_axis == 2)
            {
                move_vector.z = delta_mouse_move_uv.dotProduct(axis_z_direction_uv) * angularVelocity;
            }
            else
            {
                return;
            }

            // 构建平移矩阵并更新模型
            Matrix4x4 translate_mat;
            translate_mat.makeTransform(move_vector, Vector3::UNIT_SCALE, Quaternion::IDENTITY);
            new_model_matrix = axis_model_matrix * translate_mat;
            new_model_matrix = new_model_matrix * Matrix4x4(model_rotation);
            new_model_matrix = new_model_matrix * Matrix4x4::buildScaleMatrix(model_scale.x, model_scale.y, model_scale.z);

            // 更新变换组件
            Vector3    new_scale;
            Quaternion new_rotation;
            Vector3    new_translation;
            new_model_matrix.decomposition(new_translation, new_scale, new_rotation);

            Matrix4x4 translation_matrix = Matrix4x4::getTrans(new_translation);
            Matrix4x4 scale_matrix       = Matrix4x4::buildScaleMatrix(1.f, 1.f, 1.f);
            Matrix4x4 axis_model_matrix  = translation_matrix * scale_matrix;

            m_translation_axis.m_model_matrix = axis_model_matrix;
            m_rotation_axis.m_model_matrix    = axis_model_matrix;
            m_scale_aixs.m_model_matrix       = axis_model_matrix;

            g_editor_global_context.m_render_system->setVisibleAxis(m_translation_axis);

            transform_component->setPosition(new_translation);
            transform_component->setRotation(new_rotation);
            transform_component->setScale(new_scale);
        }
        else if (m_axis_mode == EditorAxisMode::RotateMode) // 旋转
        {
            float   last_mouse_u = (last_mouse_pos_x - engine_window_pos.x) / engine_window_size.x;
            float   last_mouse_v = (last_mouse_pos_y - engine_window_pos.y) / engine_window_size.y;
            Vector2 last_move_vector(last_mouse_u - model_origin_clip_uv.x, last_mouse_v - model_origin_clip_uv.y);
            float   new_mouse_u = (new_mouse_pos_x - engine_window_pos.x) / engine_window_size.x;
            float   new_mouse_v = (new_mouse_pos_y - engine_window_pos.y) / engine_window_size.y;
            Vector2 new_move_vector(new_mouse_u - model_origin_clip_uv.x, new_mouse_v - model_origin_clip_uv.y);
            Vector3 delta_mouse_uv_3(delta_mouse_move_uv.x, delta_mouse_move_uv.y, 0);
            float   move_radian;
            Vector3 axis_of_rotation = {0, 0, 0};
            if (cursor_on_axis == 0)
            {
                move_radian = (delta_mouse_move_uv * angularVelocity).length();
                if (m_camera->forward().dotProduct(Vector3::UNIT_X) < 0)
                {
                    move_radian = -move_radian;
                }
                axis_of_rotation.x = 1;
            }
            else if (cursor_on_axis == 1)
            {
                move_radian = (delta_mouse_move_uv * angularVelocity).length();
                if (m_camera->forward().dotProduct(Vector3::UNIT_Y) < 0)
                {
                    move_radian = -move_radian;
                }
                axis_of_rotation.y = 1;
            }
            else if (cursor_on_axis == 2)
            {
                move_radian = (delta_mouse_move_uv * angularVelocity).length();
                if (m_camera->forward().dotProduct(Vector3::UNIT_Z) < 0)
                {
                    move_radian = -move_radian;
                }
                axis_of_rotation.z = 1;
            }
            else
            {
                return;
            }
            float move_direction = last_move_vector.x * new_move_vector.y - new_move_vector.x * last_move_vector.y;
            if (move_direction < 0)
            {
                move_radian = -move_radian;
            }

            Quaternion move_rot;
            move_rot.fromAngleAxis(Radian(move_radian), axis_of_rotation);
            new_model_matrix = axis_model_matrix * move_rot;
            new_model_matrix = new_model_matrix * Matrix4x4(model_rotation);
            new_model_matrix =
                new_model_matrix * Matrix4x4::buildScaleMatrix(model_scale.x, model_scale.y, model_scale.z);
            Vector3    new_scale;
            Quaternion new_rotation;
            Vector3    new_translation;

            new_model_matrix.decomposition(new_translation, new_scale, new_rotation);

            transform_component->setPosition(new_translation);
            transform_component->setRotation(new_rotation);
            transform_component->setScale(new_scale);
            m_scale_aixs.m_model_matrix = new_model_matrix;
        }
        else if (m_axis_mode == EditorAxisMode::ScaleMode) // scale
        {
            Vector3 delta_scale_vector = {0, 0, 0};
            Vector3 new_model_scale    = {0, 0, 0};
            if (cursor_on_axis == 0)
            {
                delta_scale_vector.x = 0.01f;
                if (delta_mouse_move_uv.dotProduct(axis_x_direction_uv) < 0)
                {
                    delta_scale_vector = -delta_scale_vector;
                }
            }
            else if (cursor_on_axis == 1)
            {
                delta_scale_vector.y = 0.01f;
                if (delta_mouse_move_uv.dotProduct(axis_y_direction_uv) < 0)
                {
                    delta_scale_vector = -delta_scale_vector;
                }
            }
            else if (cursor_on_axis == 2)
            {
                delta_scale_vector.z = 0.01f;
                if (delta_mouse_move_uv.dotProduct(axis_z_direction_uv) < 0)
                {
                    delta_scale_vector = -delta_scale_vector;
                }
            }
            else
            {
                return;
            }
            new_model_scale   = model_scale + delta_scale_vector;
            axis_model_matrix = axis_model_matrix * Matrix4x4(model_rotation);
            Matrix4x4 scale_mat;
            scale_mat.makeTransform(Vector3::ZERO, new_model_scale, Quaternion::IDENTITY);
            new_model_matrix = axis_model_matrix * scale_mat;
            Vector3    new_scale;
            Quaternion new_rotation;
            Vector3    new_translation;
            new_model_matrix.decomposition(new_translation, new_scale, new_rotation);

            transform_component->setPosition(new_translation);
            transform_component->setRotation(new_rotation);
            transform_component->setScale(new_scale);
        }

        // 保存新的模型矩阵
        setSelectedObjectMatrix(new_model_matrix);
    }

    /// 上传坐标轴资源到渲染系统
    void EditorSceneManager::uploadAxisResource()
    {
        auto& instance_id_allocator   = g_editor_global_context.m_render_system->getGOInstanceIdAllocator();
        auto& mesh_asset_id_allocator = g_editor_global_context.m_render_system->getMeshAssetIdAllocator();

        // 为平移坐标轴分配ID
        {
            GameObjectPartId axis_instance_id = {0xFFAA, 0xFFAA};
            MeshSourceDesc   mesh_source_desc = {"%%translation_axis%%"};

            m_translation_axis.m_instance_id   = instance_id_allocator.allocGuid(axis_instance_id);
            m_translation_axis.m_mesh_asset_id = mesh_asset_id_allocator.allocGuid(mesh_source_desc);
        }

        // 为旋转坐标轴分配ID
        {
            GameObjectPartId axis_instance_id = {0xFFBB, 0xFFBB};
            MeshSourceDesc   mesh_source_desc = {"%%rotate_axis%%"};

            m_rotation_axis.m_instance_id   = instance_id_allocator.allocGuid(axis_instance_id);
            m_rotation_axis.m_mesh_asset_id = mesh_asset_id_allocator.allocGuid(mesh_source_desc);
        }

        // 为缩放坐标轴分配ID
        {
            GameObjectPartId axis_instance_id = {0xFFCC, 0xFFCC};
            MeshSourceDesc   mesh_source_desc = {"%%scale_axis%%"};

            m_scale_aixs.m_instance_id   = instance_id_allocator.allocGuid(axis_instance_id);
            m_scale_aixs.m_mesh_asset_id = mesh_asset_id_allocator.allocGuid(mesh_source_desc);
        }

        // 创建坐标轴渲染实体
        g_editor_global_context.m_render_system->createAxis(
            {m_translation_axis, m_rotation_axis, m_scale_aixs},
            {m_translation_axis.m_mesh_data, m_rotation_axis.m_mesh_data, m_scale_aixs.m_mesh_data});
    }

    /// 获取被点击网格的GUID
    size_t EditorSceneManager::getGuidOfPickedMesh(const Vector2& picked_uv) const
    {
        return g_editor_global_context.m_render_system->getGuidOfPickedMesh(picked_uv);
    }
}