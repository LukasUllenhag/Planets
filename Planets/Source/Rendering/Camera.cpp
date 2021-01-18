#include "Camera.h"
#include "../Keyboard.h"
#include "../Mathematics/Algorithms.h"

Camera::Camera()
{
    // Reset the cursor's position, so that the next cursor
    // movement becomes an offset
	Window::SetCursorPosition(0.0, 0.0);
}

void Camera::UpdateRotation(const double xOffset, const double yOffset)
{
	// When the user moves his/her mouse downwards, yOffset
	// increases and we want to rotate the camera clockwise
	// (hence, the minus sign in front of yOffset) around the x-axis
	mXRotation += -(float)yOffset * SENSITIVITY;

	// When the user moves his/her mouse to the right, xOffset
	// increases and we want to rotate the camera clockwise
	// (hence, the minus sign in front of xOffset) around the y-axis
	mYRotation += -(float)xOffset * SENSITIVITY;

    // Limit the camera's up/down rotation
	mXRotation = std::clamp(mXRotation, -ConvertDegreesToRadians(90.0f), ConvertDegreesToRadians(90.0f));

    // Reset the cursor's position, so that the next cursor
    // movement becomes an offset
	Window::SetCursorPosition(0.0, 0.0);
}

void Camera::UpdatePosition(const float deltaTime)
{
    // The xz - direction in which the camera points in
    Vector3 forward = GetDirectionXZ();

    // The length of "right" is guaranteed to be 1, due to the fact that
    // "forward" and "up" are perpendicular to each other and that they both have
    // a length of one
    Vector3 right = forward.Cross(vector::up);

    // The xz-direction the camera should move in
    Vector3 movementDirectionXZ;
    if (Keyboard::KeyIsPressed(GLFW_KEY_A))
    {
        movementDirectionXZ -= right;
    }
    if (Keyboard::KeyIsPressed(GLFW_KEY_D))
    {
        movementDirectionXZ += right;
    }
    if (Keyboard::KeyIsPressed(GLFW_KEY_W))
    {
        movementDirectionXZ += forward;
    }
    if (Keyboard::KeyIsPressed(GLFW_KEY_S))
    {
        movementDirectionXZ -= forward;
    }

    // The y-direction the camera should move in
    float movementDirectionY = 0.0f;

    if (Keyboard::KeyIsPressed(GLFW_KEY_SPACE))
    {
        movementDirectionY += 1.0f;
    }
    if (Keyboard::KeyIsPressed(GLFW_KEY_LEFT_SHIFT))
    {
        movementDirectionY -= 1.0f;
    }
    // The vector should be of unit length, since it shows the 
    // direction of the movement
    movementDirectionXZ.Normalize();

    // The reason why we separated the y-movement from the xz-movement was to be able to
    // normalize the vectors individually. We do not want the velocity in the y-direction to affect the 
    // velocity in the xz-direction.
    Vector3 movementDirection = movementDirectionXZ + vector::up * movementDirectionY;

    mPosition += movementDirection * MOVEMENT_SPEED * deltaTime;
}


const Vector3& Camera::GetPosition() const
{
	return mPosition;
}

float Camera::GetXRotation() const
{
	return mXRotation;
}

float Camera::GetYRotation() const
{
	return mYRotation;
}

Vector3 Camera::GetDirectionXZ() const
{

	return Vector3(-sin(mYRotation), 0.0f, -cos(mYRotation));

}
