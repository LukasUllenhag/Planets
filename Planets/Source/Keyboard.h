#pragma once
#include "Window/Window.h"

class Keyboard
{
public:
	Keyboard(Window& window);
	static bool KeyIsPressed(int key);
private:
	static inline GLFWwindow* msGlfwWindow = nullptr;
};