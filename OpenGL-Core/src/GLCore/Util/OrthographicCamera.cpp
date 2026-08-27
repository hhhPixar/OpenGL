/*
 * OrthographicCamera.cpp —— 正交投影相机的实现
 *
 * 在引擎中的角色：实现投影矩阵的建立和视图矩阵的重算。
 *   - 投影：glm::ortho(left,right,bottom,top,near,far) 生成正交投影矩阵，
 *     它把世界坐标矩形 [left,right]x[bottom,top] 线性映射到 NDC 的 [-1,1]^2。
 *     注意 OrthographicCamera 是 2D 用，所以这里 near/far 直接用 -1/1（几乎无深度含义）。
 *   - 视图：先构造一个"把相机放到 m_Position 并绕 Z 轴旋转 m_Rotation 的变换 transform"，
 *     View = inverse(transform)——因为视图矩阵是"世界->相机"的变换，正是相机变换的逆。
 *
 * 关键依赖：glm/gtc/matrix_transform.hpp 提供 glm::ortho/translate/rotate。
 */
#include "glpch.h"
#include "OrthographicCamera.h"

#include <glm/gtc/matrix_transform.hpp>

namespace GLCore::Utils {

	OrthographicCamera::OrthographicCamera(float left, float right, float bottom, float top)
		// 构造函数初始化列表：
		//   - glm::ortho(left,right,bottom,top,-1,1) 返回正交投影矩阵；
		//   - m_ViewMatrix(1.0f) 表示单位矩阵（无任何移动/旋转，相机在原点）。
		// 初始化列表比在函数体里赋值更高效——直接构造，少一次默认构造 + 拷贝。
		: m_ProjectionMatrix(glm::ortho(left, right, bottom, top, -1.0f, 1.0f)), m_ViewMatrix(1.0f)
	{
		// 组合出 ViewProjection。Projection * View 的顺序不能颠倒——
		// 矩阵乘法从右到左作用：先把顶点用 View 变到相机空间，再用 Projection 投到 NDC。
		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}

	// 运行时重设投影范围。
	void OrthographicCamera::SetProjection(float left, float right, float bottom, float top)
	{
		// 重新生成正交投影矩阵。
		m_ProjectionMatrix = glm::ortho(left, right, bottom, top, -1.0f, 1.0f);
		// 投影范围改了之后，ViewProjection 也要跟着重算（View 没变也要重乘一次）。
		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}

	// 重算 View：根据当前位置和旋转角构造相机变换，再取逆得到视图矩阵。
	void OrthographicCamera::RecalculateViewMatrix()
	{
		// 构造"相机自身的世界变换"：
		//   translate(单位矩阵, m_Position) —— 在世界里把相机移到 m_Position；
		//   rotate(单位矩阵, 弧度, 轴) —— 绕 Z 轴(0,0,1) 旋转，2D 场景里绕 Z 转=就是画面上的旋转。
		// glm::rotate 的角度参数要弧度，glm::radians 把度数转弧度（例如 180 度 -> pi）。
		// 两个变换相乘：先 rotate 后 translate（写在 translate(...) * rotate(...) 中，
		// 因为 OpenGL/glm 用列主序矩阵，右边的先作用——即顶点先被旋转再被平移）。
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), m_Position) *
			// 绕 Z 轴旋转 m_Rotation 度（已用 glm::radians 转弧度）。
			glm::rotate(glm::mat4(1.0f), glm::radians(m_Rotation), glm::vec3(0, 0, 1));

		// View 是 transform 的逆——把整个世界反向变换，使相机正好落到原点、朝向对齐 Z 轴。
		// 这正是"视图矩阵 = 相机世界变换的逆"的含义。
		m_ViewMatrix = glm::inverse(transform);
		// ViewProjection 也随之更新。
		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}

}