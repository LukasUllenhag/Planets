#include "Game.h"
#include "Benchmark/BenchmarkMacros.h"
#include "Rendering/GlMacro.h"
#include "Console/Log.h"

Game::Game()
    :
    mWindow("Water", 1920, 1080),
    mKeyboard(mWindow),
    mProjectionMatrix(matrix::GetProjection(ConvertDegreesToRadians(60.0f), 0.1f, 200.0f)),
    mPostProcessor(std::bind(&Game::RenderWithPostProcessingEffect, this))
{
    NAME_THREAD("Main");
    BENCHMARK;
    mWindow.SetCloseCallback(std::bind(&Game::CloseWindowCallback, std::ref(*this)));
    mWindow.SetCursorCallback(std::bind(&Camera::UpdateRotation, std::ref(mCamera), 
        std::placeholders::_1, std::placeholders::_2));

    SetGlStates();

    mPostProcessor.AddEffect("WaterEffect");
    mPostProcessor.AddEffect("NoEffect");

    // Start the timer
    mTimer.Time();
}

Game::~Game()
{
    // Destructors should not throw exception, hence no GL macro
    glDeleteVertexArrays(1, &mVao);
    SAVE_BENCHMARK;
}

void Game::BeginLoop()
{
    // Loop until the user closes the window
    while (!mWindowShouldClose)
    {
        Loop();
    }
}

void Game::Loop()
{
    BENCHMARK;

    GL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

    Update();
    Render();

    mDeltaTime = (float)mTimer.Time();

    NAMED_BENCHMARK("Swap and poll");
    // Swap front and back buffers
    mWindow.SwapBuffers();

    // Poll for and process events
    mWindow.PollEvents();
}

void Game::Update()
{
    BENCHMARK;
    // LOG(1.0f / mDeltaTime << std::endl);

    mTime += (double)mDeltaTime;
   
    mCamera.UpdatePosition(mDeltaTime);
}

void Game::Render() const
{
    BENCHMARK;

    mPostProcessor.Render("NoEffect");
}

void Game::RenderWithPostProcessingEffect()
{
}

void Game::CloseWindowCallback()
{
    // Close the window
    mWindowShouldClose = true;
}

void Game::SetGlStates()
{
    // This vertex array object will not contain any states. 
    // Its only purpose is to fulfill some GPUs requirement 
    // to always have a vertex array object bound.
    GL(glCreateVertexArrays(1, &mVao));
    GL(glBindVertexArray(mVao));

    GL(glClearColor(135.0f / 255.0f, 206.0f / 255.0f, 235.0f / 255.0f, 0.0f));
    GL(glEnable(GL_DEPTH_TEST));
    GL(glEnable(GL_CULL_FACE));
    GL(glEnable(GL_BLEND));
    GL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
}