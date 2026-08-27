/*
 * OrthographicCamera.h —— 正交投影相机的声明
 *
 * 在引擎中的角色：2D 渲染用的相机。相机的作用是把"世界坐标"转换成"屏幕坐标"，
 * 这一过程通过两个矩阵完成：
 *   - 投影矩阵(Projection)：定义可见范围（左右上下边界），用 glm::ortho 生成。
 *     它把世界矩形框 [left,right]x[bottom,top] 映射到 OpenGL 标准设备坐标(NDC)的 [-1,1]^2。
 *   - 视图矩阵(View)：描述相机的位置和旋转，本质是"世界->相机空间"的逆变换。
 *   - 最终用 ViewProjection = Projection * View 把顶点直接送到 NDC。
 *
 * 关键依赖：glm/glm.h 提供的 vec3/mat4 数学类型和后续矩阵函数。
 *
 * 使用方式：构造时给定 left/right/bottom/top 设投影；用 SetPosition/SetRotation 移动相机，
 * 内部会自动重算 View 和 ViewProjection。
 */
#pragma once

#include <glm/glm.hpp>

namespace GLCore::Utils {

	// 正交投影相机：2D 专用，无透视形变（远处和近处一样大）。
	// 对外暴露投影/视图/最终矩阵的只读访问，以及位置、角度的读写接口。
	class OrthographicCamera
	{
	public:
		// 构造时给定可见范围的左右上下边界，内部即初始化投影矩阵。
		OrthographicCamera(float left, float right, float bottom, float top);

		// 运行时改投影范围（例如窗口尺寸变化、缩放时）。参数含义同构造函数。
		void SetProjection(float left, float right, float bottom, float top);

		// Get 方法返回 const 引用：const glm::vec3& 表示"返回对象的引用，且不允许修改"。
		// 返回引用避免了拷贝整个 vec3（虽然小，但养成习惯）；const 表示只读访问。
		const glm::vec3& GetPosition() const { return m_Position; }
		// Set 方法：赋值后立即重算 View 矩阵——保证外部拿到的 ViewProjection 总是最新的。
		void SetPosition(const glm::vec3& position) { m_Position = position; RecalculateViewMatrix(); }

		// 返回当前旋转角度（度数）。
		float GetRotation() const { return m_Rotation; }
		// 设置旋转角度后同样立即重算 View。
		void SetRotation(float rotation) { m_Rotation = rotation; RecalculateViewMatrix(); }

		// 三个矩阵的只读访问。渲染时 VertexArray + shader + ViewProjection 一起提交给 OpenGL。
		const glm::mat4& GetProjectionMatrix() const { return m_ProjectionMatrix; }
		// 视图矩阵：世界->相机空间的变换。
		const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }
		// 最终矩阵：Projection * View，shader 里直接用它把顶点变到 NDC。
		const glm::mat4& GetViewProjectionMatrix() const { return m_ViewProjectionMatrix; }
	private:
		// 根据 m_Position/m_Rotation 重算 View 和 ViewProjection。在位置/旋转改变时被调用。
		void RecalculateViewMatrix();
	private:
		// 三个矩阵均为 4x4。成员声明顺序就是构造函数初始化列表里要写的顺序。
		glm::mat4 m_ProjectionMatrix;
		glm::mat4 m_ViewMatrix;
		glm::mat4 m_ViewProjectionMatrix;

		// 默认成员初始化(C++11)：声明处直接给 = {…} 或 = 0.0f，构造时若没显式初始化就用这个值。
		// 这样即使新增构造函数忘了写，成员也有确定初值，不会是脏数据。
		glm::vec3 m_Position = { 0.0f, 0.0f, 0.0f };
		// 相机绕 Z 轴的旋转角度（度数），RecalculateViewMatrix 里会转成弧度交给 glm::rotate。
		float m_Rotation = 0.0f;
	};

}
