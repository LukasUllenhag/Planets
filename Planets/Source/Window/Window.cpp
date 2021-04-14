#include "Window.h"
#include "../CustomException.h"

Window::Window(const std::string& title, int width, int height)
	:
	mWidth(width),
	mHeight(height)
{
	// There should only be one instance of this class
	assert(!msWindow);
	msWindow = this;

    InitializeGLFW(title);
    InitializeGLEW();

    glfwSetWindowCloseCallback(mGlfwWindow, CloseCallback);

    glfwSetInputMode(mGlfwWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(mGlfwWindow, CursorCallback);
}

Window::~Window()
{
    glfwTerminate();
}

void Window::SetCloseCallback(std::function<void()> callback)
{
    mCloseCallback = callback;
}

void Window::SetCursorCallback(std::function<void(double, double)> callback)
{
    msWindow->mCursorCallback = callback;
}

void Window::CloseCallback(GLFWwindow* window)
{
    msWindow->mCloseCallback();
}

void Window::CursorCallback(GLFWwindow* window, const double xPosition, const double yPosition)
{
    msWindow->mCursorCallback(xPosition, yPosition);
}

void Window::InitializeGLFW(const std::string& title)
{
    // Initialize GLFW
    if (!glfwInit())
    {
        throw CREATE_CUSTOM_EXCEPTION("Failed to initialize GLFW");
    }

    // The user should not be able to resize the window
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    // Create the window 
    mGlfwWindow = glfwCreateWindow(mWidth, mHeight, title.c_str(), NULL, NULL);
    if (!mGlfwWindow)
    {
        glfwTerminate();
        throw CREATE_CUSTOM_EXCEPTION("Failed to create the GLFW window");
    }

    // Make the window's context current
    glfwMakeContextCurrent(mGlfwWindow);
}

void Window::InitializeGLEW() const
{
    GLenum status = glewInit();
    if (GLEW_OK != status)
    { 
        std::string errorString = reinterpret_cast<const char*>(glewGetErrorString(status));
        throw CREATE_CUSTOM_EXCEPTION("Failed to initialize GLEW\n" + errorString);
    }
}

GLFWwindow* Window::GetGlfwWindow()
{
    return mGlfwWindow;
}

int Window::GetWidth()
{
    assert(msWindow);
    return msWindow->mWidth;
}

int Window::GetHeight()
{
    assert(msWindow);
    return msWindow->mHeight;
}

void Window::SetCursorPosition(const double xPosition, const double yPosition)
{
    glfwSetCursorPos(msWindow->mGlfwWindow, xPosition, yPosition);
}

bool Window::ShouldClose() const
{
    return glfwWindowShouldClose(mGlfwWindow);
}

void Window::SwapBuffers() const
{
    glfwSwapBuffers(mGlfwWindow);
}

void Window::PollEvents() const
{
    glfwPollEvents();
}
