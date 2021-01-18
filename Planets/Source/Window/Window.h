#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>

class WindowAccessSpecifier;

// Singleton
class Window
{
public:
	Window(const std::string& title, int width, int height);
	// One should not be able to copy nor move a "Window" instance
	Window(const Window& other) = delete;
	Window& operator=(const Window& other) = delete;

	~Window();

	static int GetWidth();
	static int GetHeight();
	static void SetCursorPosition(double xPosition, double yPosition);

	bool ShouldClose() const;
	void SetCloseCallback(std::function<void()> callback);
	void SetCursorCallback(std::function<void(double, double)> callback);

	void SwapBuffers() const;
	void PollEvents() const;
private:
	static void CloseCallback(GLFWwindow* window);
	static void CursorCallback(GLFWwindow* window, double xPosition, double yPosition);

	void InitializeGLFW(const std::string& title);
	void InitializeGLEW() const;
	GLFWwindow* GetGlfwWindow();
private:
	friend class WindowAccessSpecifier;
	// We store a static version of the only object of this class, 
	// in order to be able to access the width and height statically
	static inline Window* msWindow = nullptr;

	GLFWwindow* mGlfwWindow = nullptr;
	std::function<void()> mCloseCallback = [](){};
	std::function<void(double, double)> mCursorCallback = [](double, double) {};

	int mWidth = 0;
	int mHeight = 0;
};