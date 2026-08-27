/*
 * 文件: ExampleLayer.cpp
 * 角色: ExampleLayer 类的实现。这是整个项目最值得学习的文件 ——
 *       它完整展示了一条 OpenGL 渲染管线从资源分配到逐帧绘制的全过程:
 *       顶点数据 -> VBO(存顶点) -> VAO(描述属性布局) -> IBO(索引拼三角形)
 *       -> 着色器 program -> 设置 uniform(视图投影矩阵 + 颜色) -> glDrawElements 画出四边形。
 * 关键依赖: ExampleLayer.h(本层声明)、GLCore 工具(Shader、相机控制器、事件系统)、glm、ImGui。
 * 着色器路径: "assets/shaders/test.vert.glsl" 与 test.frag.glsl —— 相对于"运行时的当前工作目录"
 *            (即可执行文件被启动时所在目录),所以资源要和 exe 放在一起或从对应目录启动。
 */

#include "ExampleLayer.h"

// 把两个命名空间的名字直接引入,后续写 Shader、Event、Timestep 等更简洁。
// using namespace 是"命名空间别名"用法,初学者提示:.cpp 里用比较安全,头文件里一般避免。
using namespace GLCore;
using namespace GLCore::Utils;

// 构造函数:用初始化列表给 m_CameraController 传宽高比 16:9。
// 正交相机控制器需要知道视口的宽高比,才能正确设置正交投影矩阵的缩放,保证画面不被拉伸。
ExampleLayer::ExampleLayer()
	: m_CameraController(16.0f / 9.0f)
{

}

// 析构函数:这里没有手动释放 m_Shader/VAO/VBO/IBO —— 那些资源在 OnDetach 里释放。
// 留空实现是因为基类析构是 virtual,需要有个实现体;真正清理在 OnDetach 完成。
ExampleLayer::~ExampleLayer()
{

}

// OnAttach:层被加入层栈时调用一次,负责"分配资源 + 设置 GL 状态 + 加载着色器"。
void ExampleLayer::OnAttach()
{
	// 打开 OpenGL 的调试输出(遇到 GL 错误时打印诊断信息),方便开发期发现问题。
	EnableGLDebugging();

	// 开启深度测试:绘制时根据片元的深度决定前后遮挡,避免后面的三角形错误地盖住前面的。
	glEnable(GL_DEPTH_TEST);
	// 开启混合:让带 alpha(透明度)的颜色能够和背景按比例混合,实现半透明效果。
	glEnable(GL_BLEND);
	// 设置混合因子:源(新片元)用它的 alpha、目标(已存在)用 1-alpha,
	// 这是经典的"normal alpha blending"公式:out = src*srcAlpha + dst*(1-srcAlpha)。
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// 从两个 GLSL 文本文件加载并编译/链接成一个着色器 program,返回其句柄。
	// test.vert.glsl 是顶点着色器(决定每个顶点的位置),test.frag.glsl 是片元着色器(决定每个像素的颜色)。
	m_Shader = Shader::FromGLSLTextFiles(
		"assets/shaders/test.vert.glsl",
		"assets/shaders/test.frag.glsl"
	);

	// ---- 顶点数组对象(VAO)---- 创建并绑定。VAO 会"记录"后续对顶点属性的所有状态设置,
	//      以后只要重新 glBindVertexArray 这个 VAO,就能恢复整套属性配置,无需重新设置。
	glCreateVertexArrays(1, &m_QuadVA);
	glBindVertexArray(m_QuadVA);

	// 四边形的 4 个顶点,每个顶点 3 个分量(x,y,z)。注意 z 都是 0,即四边形在 z=0 平面上。
	// 顶点顺序:左下 -> 右下 -> 右上 -> 左上(按逆时针看,便于后面用索引拼两个三角形)。
	float vertices[] = {
		-0.5f, -0.5f, 0.0f,
		 0.5f, -0.5f, 0.0f,
		 0.5f,  0.5f, 0.0f,
		-0.5f,  0.5f, 0.0f
	};

	// ---- 顶点缓冲对象(VBO)---- 在 GPU 上开辟一块缓冲区,把顶点数据传上去。
	//      GL_ARRAY_BUFFER 表示"这块缓冲用作顶点属性数据源"。
	glCreateBuffers(1, &m_QuadVB);
	glBindBuffer(GL_ARRAY_BUFFER, m_QuadVB);
	// sizeof(vertices) 是整个数组的字节数;GL_STATIC_DRAW 提示 GPU"这些数据基本不变",驱动会放到适合静态读取的内存。
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// 启用第 0 号顶点属性(对应顶点着色器里 layout(location=0) 的 a_Position)。
	glEnableVertexAttribArray(0);
	// 详细描述第 0 号属性如何从 VBO 里取数据:
	//   参数:属性索引0、每顶点3个分量、类型GL_FLOAT、不归一化GL_FALSE、
	//         每顶点步进 sizeof(float)*3(即12字节,因为每顶点3个float)、起始偏移0。
	// 这样 GL 就知道:每画一个顶点,从 VBO 当前位置读 3 个 float 当作位置传给着色器。
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, 0);

	// 索引数组:用 0,1,2 组成第一个三角形(左下-右下-右上),
	//          用 2,3,0 组成第二个三角形(右上-左上-左下),两个三角形拼成正方形。
	// 用索引的好处:复用顶点,4 个顶点就能画两个三角形,而不用列 6 个顶点。
	uint32_t indices[] = { 0, 1, 2, 2, 3, 0 };
	// ---- 索引缓冲对象(IBO)---- 绑到 GL_ELEMENT_ARRAY_BUFFER。
	//      重要:IBO 必须在当前 VAO 已绑定时绑定,VAO 才会把它"记住",之后 glDrawElements 才能用到这些索引。
	glCreateBuffers(1, &m_QuadIB);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_QuadIB);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
}

// OnDetach:层被移出层栈时调用一次,负责释放 OnAttach 里分配的 GPU 资源。
// 这是"手动版的 RAII":不像 C++ 对象自动析构,GL 对象要主动 glDelete 才回收。
void ExampleLayer::OnDetach()
{
	glDeleteVertexArrays(1, &m_QuadVA);  // 删除顶点数组对象
	glDeleteBuffers(1, &m_QuadVB);       // 删除顶点缓冲对象
	glDeleteBuffers(1, &m_QuadIB);      // 删除索引缓冲对象
}

// OnEvent:有输入事件(鼠标移动/点击、键盘、窗口尺寸变化等)时由引擎分派到此。
// 参数 event 是一个 Event 基类引用,具体可能是 MouseButtonPressedEvent 等子类。
void ExampleLayer::OnEvent(Event& event)
{
	// 先把事件交给相机控制器:它内部会处理鼠标拖动(平移)、滚轮(缩放)等,更新相机。
	m_CameraController.OnEvent(event);

	// 事件分派器:用来"按类型"分发事件,比一串 if-else 判断类型更清晰。
	EventDispatcher dispatcher(event);
	// Dispatch<具体事件类型>(回调) 的语义:如果当前事件正好是该类型,就调用回调处理它。
	// 回调返回 bool 表示"是否已处理该事件"(返回 true 通常表示不再继续传播)。

	// 鼠标按下事件:把当前颜色切到"备用色",实现"按住变蓝"的效果。
	dispatcher.Dispatch<MouseButtonPressedEvent>(
		[&](MouseButtonPressedEvent& e)
		{
			// [&] 是 lambda 的"捕获列表":[&] 表示按引用捕获外部所有变量(这里是 m_SquareAlternateColor/m_SquareColor)。
			// lambda 是匿名函数,这里就地写一个小回调,访问本对象的成员。
			m_SquareColor = m_SquareAlternateColor;
			return false;  // 返回 false:表示不拦截事件,让它继续传播给其它层/处理器
		});
	// 鼠标松开事件:把颜色切回"基础色"。
	dispatcher.Dispatch<MouseButtonReleasedEvent>(
		[&](MouseButtonReleasedEvent& e)
		{
			m_SquareColor = m_SquareBaseColor;
			return false;
		});
}

// OnUpdate:每帧调用一次,负责"更新逻辑 + 渲染"。
// 参数 ts 是本帧的时间步长(秒),用于做与帧率无关的平滑运动。
void ExampleLayer::OnUpdate(Timestep ts)
{
	// 让相机控制器按本帧时间处理键盘移动等更新(比如 WASD 平移相机)。
	m_CameraController.OnUpdate(ts);

	// 用深灰色清屏色清空颜色缓冲,同时清空深度缓冲(深度测试需要每帧重置深度)。
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// 激活着色器程序:后续的 uniform 设置和绘制都作用于这个 program。
	glUseProgram(m_Shader->GetRendererID());

	// 设置 uniform "u_ViewProjection"(视图投影矩阵)。
	// 先用 glGetUniformLocation 查询这个 uniform 在程序里的"位置"(一个 int 下标),
	// 然后用 glUniformMatrix4fv 把一个 4x4 矩阵传进去。
	int location = glGetUniformLocation(m_Shader->GetRendererID(), "u_ViewProjection");
	// glm::value_ptr 把 glm::mat4 转成 OpenGL 需要的连续 float* 指针;
	// 参数:位置、数量1、是否转置 GL_FALSE(列主序,glm 默认列主序)、数据指针。
	glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(m_CameraController.GetCamera().GetViewProjectionMatrix()));

	// 同样地设置颜色 uniform "u_Color",这是一个 vec4,用 glUniform4fv 传 4 个 float。
	location = glGetUniformLocation(m_Shader->GetRendererID(), "u_Color");
	glUniform4fv(location, 1, glm::value_ptr(m_SquareColor));

	// 绑定要用的 VAO(它会同时带出对应的 VBO 属性配置和 IBO 索引),然后画。
	glBindVertexArray(m_QuadVA);
	// 用索引绘制:模式 GL_TRIANGLES(每 3 个索引拼一个三角形)、画 6 个索引、索引类型 GL_UNSIGNED_INT、
	//            最后一个参数 nullptr 表示"索引在当前绑定的 IBO 里"(而非另外的指针)。
	// 6 个索引 -> 2 个三角形 -> 一个四边形。
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
}

// OnImGuiRender:每帧的 ImGui 阶段调用,用于画调试/控制面板(即时模式 GUI)。
void ExampleLayer::OnImGuiRender()
{
	// ImGui::Begin/End 之间定义一个名为 "Controls" 的窗口。
	ImGui::Begin("Controls");
	// ColorEdit4 是一个 RGBA 颜色选择控件,直接读写传入的 float[4];
	// 这里用 glm::value_ptr 把 glm::vec4 转成指针交给 ImGui。
	// 如果用户改了"基础色",即时把当前颜色也同步成基础色(这样松开鼠标后立刻看到新基础色)。
	if (ImGui::ColorEdit4("Square Base Color", glm::value_ptr(m_SquareBaseColor)))
		m_SquareColor = m_SquareBaseColor;
	// 备用色只编辑存储,不立刻应用(下次按下鼠标时才用到)。
	ImGui::ColorEdit4("Square Alternate Color", glm::value_ptr(m_SquareAlternateColor));
	ImGui::End();
}
