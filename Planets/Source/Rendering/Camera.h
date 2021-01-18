#pragma once
#include "../Mathematics/Vector/Vector.h"
class Camera
{
public:
	Camera();
	void UpdateRotation(double xOffset, double yOffset);
	void UpdatePosition(float deltaTime);
	const Vector3& GetPosition() const;

	float GetXRotation() const;
	float GetYRotation() const;
private:
	// Returns the xz-direction in which the camera points in. The
	// y-value i always 0
	Vector3 GetDirectionXZ() const;
private:
	Vector3 mPosition;
	// The rotation, in radians, counterclockwise around the x-axis
	float mXRotation = 0.0f;
	// The rotation, in radians, counterclockwise around the y-axis
	float mYRotation = 0.0f;

	static constexpr float MOVEMENT_SPEED = 5.0f;
	static constexpr float SENSITIVITY = 0.001f;
};