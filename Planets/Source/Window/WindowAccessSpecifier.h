#pragma once
#include "Window.h"

class Keyboard;

// Specifies which classes should have access to a
// chosen subset (in this case only the GLFW window)
// of the private members of "Window"
class WindowAccessSpecifier
{
public:
	static GLFWwindow* AccessGlfwWindow(Window& window);
private:
	// Friend the classes that should have access to the 
	// GLFW window
	friend class Keyboard;
};
