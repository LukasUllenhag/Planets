#pragma once
#include "Window/Window.h"
#include "Keyboard.h"
#include "Rendering/Program.h"
#include "Rendering/Camera.h"
#include "Mathematics/Matrix/Matrix.h"
#include "Timer.h"
#include "Rendering/PostProcessing/PostProcessor.h"
#include "CelestialBody/CelestialBody.h"
#include "DynamicVariableManager.h"

class Game
{
public:
	Game();
	~Game();

	void BeginLoop();
private:
	void Loop();
	void Update();
	void Render() const;
	void RenderWithPostProcessingEffect();
	void CloseWindowCallback();
	void SetGlStates();
private:
	Window mWindow;
	Keyboard mKeyboard;
	Camera mCamera;
	bool mWindowShouldClose = false;

	Timer mTimer;
	// Make the delta time for the first frame
	// 1/60 s, i.e., 60 FPS
	float mDeltaTime = 1.0f / 60.0f;
	double mTime = 0.0f;

	// Some GPUs require that we have a vertex array 
	// object bound and this vertex array object exists
	// solely to fulfill that requirement
	GLuint mVao = 0;

	Matrix4 mProjectionMatrix;
	PostProcessor mPostProcessor;

	// Generates the terrain of the celestial body
	std::shared_ptr<Program> mCelestialBodyGeneratorProgram;
	std::shared_ptr<Program> mMoonTextureRenderingProgram;
	std::shared_ptr<Program> mMoonColourRenderingProgram;
	std::shared_ptr<Program> mPlanetRenderingProgram;
	
	DynamicVariableManager<float> mDynamicVariableManager;

	CelestialBody mTexturedMoon;
	CelestialBody mAsteroidMoon;
	CelestialBody mPlanet;
};