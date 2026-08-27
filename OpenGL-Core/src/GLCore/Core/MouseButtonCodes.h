#pragma once
/* ============================================================================
 * MouseButtonCodes.h —— 鼠标按键码定义
 *
 * 和 KeyCodes.h 同理:把 GLFW 的鼠标按键常量原样搬过来,前缀改成 HZ_,
 * 数值和 GLFW_MOUSE_BUTTON_* 完全一样(0=左键,1=右键,2=中键,3~7 是
 * 部分鼠标上的额外侧键)。
 *
 * 保持平台无关:Core 层不依赖 GLFW 头文件,用这套抽象常量;WindowsInput
 * 实现时直接把这些值传给 glfwGetMouseButton(数值相等)。
 *
 * HZ_MOUSE_BUTTON_LAST 标记按键码的上界,方便遍历/校验。
 * HZ_MOUSE_BUTTON_LEFT/RIGHT/MIDDLE 是给常用三个键起的语义别名。
 * ========================================================================== */

// From glfw3.h
#define HZ_MOUSE_BUTTON_1         0
#define HZ_MOUSE_BUTTON_2         1
#define HZ_MOUSE_BUTTON_3         2
#define HZ_MOUSE_BUTTON_4         3
#define HZ_MOUSE_BUTTON_5         4
#define HZ_MOUSE_BUTTON_6         5
#define HZ_MOUSE_BUTTON_7         6
#define HZ_MOUSE_BUTTON_8         7
#define HZ_MOUSE_BUTTON_LAST      HZ_MOUSE_BUTTON_8
#define HZ_MOUSE_BUTTON_LEFT      HZ_MOUSE_BUTTON_1
#define HZ_MOUSE_BUTTON_RIGHT     HZ_MOUSE_BUTTON_2
#define HZ_MOUSE_BUTTON_MIDDLE    HZ_MOUSE_BUTTON_3