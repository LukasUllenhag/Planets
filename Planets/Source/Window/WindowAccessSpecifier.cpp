#include "WindowAccessSpecifier.h"

GLFWwindow* WindowAccessSpecifier::AccessGlfwWindow(Window& window)
{
    return window.GetGlfwWindow();
}
