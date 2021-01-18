#include "Keyboard.h"
#include "Window/WindowAccessSpecifier.h"

Keyboard::Keyboard(Window& window)
{
	// There should only be one instance of this class
	assert(!msGlfwWindow);

	msGlfwWindow = WindowAccessSpecifier::AccessGlfwWindow(window);
}

bool Keyboard::KeyIsPressed(int key)
{
	assert(msGlfwWindow);

	return glfwGetKey(msGlfwWindow, key) == GLFW_PRESS;
}
