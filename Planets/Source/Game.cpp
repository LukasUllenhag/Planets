#include "Game.h"
#include "Benchmark/BenchmarkMacros.h"
#include "Rendering/GlMacro.h"
#include "Console/Log.h"
#include "Configure.h"

using namespace std::literals::string_literals;
Game::Game()
    :
    mWindow("Planets"s + " " + VERSION_STRING, 1920, 1080),
    mKeyboard(mWindow),
    mProjectionMatrix(matrix::GetProjection(ConvertDegreesToRadians(60.0f), 0.1f, 200.0f)),
    mPostProcessor(std::bind(&Game::RenderWithPostProcessingEffect, this)),
    mMoonTextureRenderingProgram(std::make_shared<Program>("MoonTexture")),
    mMoonColourRenderingProgram(std::make_shared<Program>("MoonColour")),
    mPlanetRenderingProgram(std::make_shared<Program>("Planet")),
    mCelestialBodyGeneratorProgram(std::make_shared<Program>("CelestialBodyGeneration")),
    mDynamicVariableManager({"Moon", "AsteroidMoon", "Planet"}),
    mTexturedMoon(mMoonTextureRenderingProgram, mCelestialBodyGeneratorProgram, {0.0f, 0.0f, -20.0f},
        10.0f, 0.02f, mDynamicVariableManager.GetGroup("Moon")),
    mAsteroidMoon(mMoonColourRenderingProgram, mCelestialBodyGeneratorProgram, { 25.0f, 0.0f, -20.0f },
        10.0f, 0.02f, mDynamicVariableManager.GetGroup("AsteroidMoon")),
    mPlanet(mPlanetRenderingProgram, mCelestialBodyGeneratorProgram, { -25.0f, 0.0f, -20.0f },
        10.0f, 0.02f, mDynamicVariableManager.GetGroup("Planet"))
{
    NAME_THREAD("Main");
    BENCHMARK;
    mWindow.SetCloseCallback(std::bind(&Game::CloseWindowCallback, std::ref(*this)));
    mWindow.SetCursorCallback(std::bind(&Camera::UpdateRotation, std::ref(mCamera), 
        std::placeholders::_1, std::placeholders::_2));

    SetGlStates();

    mPostProcessor.AddEffect("NoEffect", PostProcessingEffect(Program("NoEffect"), 
        [](GLuint colourTexture, GLuint depthTexture)
        {
            GL(glBindTextureUnit(0, colourTexture));
            GL(glBindTextureUnit(1, depthTexture));
        }));

    mPostProcessor.AddEffect("OceanEffect", PostProcessingEffect(Program("OceanEffect"),
        [
            this,
            // In order to make the normal map copyable,
            // we need to store it as a "std::shared_ptr"
            normalMap = std::make_shared<Texture>("WaterNormal"), 
            perlinNoise = PerlinNoise<2>(std::make_shared<PermutationTable<256>>())
        ]
        (GLuint colourTexture, GLuint depthTexture)
        {
            GL(glBindTextureUnit(0, colourTexture));
            GL(glBindTextureUnit(1, depthTexture));
      
            normalMap->Bind(2);

            // Calculate the projection matrix parameters. These will be used
            // for reverting the projection matrix's transformation, in order
            // to get the pixel's screen position back into its world
            // position. Calculating the parameters here, in this way, is bad
            // for multiple reasons:
            // 1) These parameters need to match the parameters used when constructing
            // "mProjectionMatrix". If we later change, for example, the near value used
            // for constructing the projection matrix, the parameters will not match and
            // we will get incorrect results.
            // 2) We are calculating the parameters every frame, when we only need to do
            // the calculation at the start of the program (at the moment, the parameters
            // do not change after they have been initialized, since we are not yet supporting
            // window resizing.
            // 3) Duplication of code (code that does the same calculations can be found inside
            // the function "matrix::GetProjection").
            // Due to all these issues, the below provisional code block will likely change 
            // in the future.
            const float near = 0.1f;
            const float far = 200.0f;

            const float top = (float)tan(ConvertDegreesToRadians(60.0f) / 2.0) * near;
            const float bottom = -top;
            const float aspectRatio = (float)Window::GetWidth() / (float)Window::GetHeight();
            const float right = aspectRatio * top;
            const float left = -right;
            // ^^^ Calculation of projection matrix parameters ^^^

            GL(glUniform1f(0, near));
            GL(glUniform1f(1, far));
            GL(glUniform1f(2, left));
            GL(glUniform1f(3, right));
            GL(glUniform1f(4, top));
            GL(glUniform1f(5, bottom));


            GL(glUniform1f(6, mPlanet.GetRadius()));

            Matrix3 viewRotation = matrix::GetRotation(-mCamera.GetXRotation(), -mCamera.GetYRotation(), 0.0f);
            // The view rotation matrix and the inverse of the view rotation matrix,
            // both need to be 4x4 matrices, since that is how the shader wants them
            Matrix4 viewRotationMatrix4 = Matrix4(viewRotation);
            std::optional<Matrix4> viewRotationInverseMatrix4 = viewRotationMatrix4.GetInverse();

            // The position of the planet, as seen from the camera
            Vector3 planetViewPosition = mPlanet.GetPosition();
            planetViewPosition -= mCamera.GetPosition();
            planetViewPosition = viewRotation * planetViewPosition;

            GL(glUniform3fv(7, 1, planetViewPosition.GetPointerToData()));
            GL(glUniformMatrix4fv(8, 1, GL_FALSE, viewRotationMatrix4.GetPointerToData()));

            // Make sure that the inversion succeeded
            assert(viewRotationInverseMatrix4);
            GL(glUniformMatrix4fv(9, 1, GL_FALSE, viewRotationInverseMatrix4->GetPointerToData()));

            // vvv Normal map rotation calculation vvv

            // We want to rotate the normal map randomly, based on 
            // the current time, to make it look like the normals
            // are moving (simulating ocean waves). We use perlin
            // noise, with the frequency "frequency" to sample random 
            // rotations.
            const float frequency = 0.5f;

            // This is the maximum amount (in radians) the normal
            // map should be able to get rotated
            const float maxRotationAmount = 0.03f;

            // Get a random rotation amount around the x-axis, for
            // the first normal map rotation matrix
            const float xRotation0 = 
                perlinNoise.Get(Vector2((float)mTime * frequency, (float)mTime * frequency))
                * maxRotationAmount;
            // Get a random rotation amount around the y-axis, for
            // the first normal map rotation matrix. Note that we are
            // offsetting the noise so that we are not getting the same
            // random value again.
            const float yRotation0 = 
                perlinNoise.Get(Vector2((float)mTime * frequency + 2.0f, (float)mTime * frequency + 4.0f))
                * maxRotationAmount;

            // Do the same thing for the second normal map rotation matrix,
            // but offset the rotation by 180° (pi radians), in order to make 
            // the two rotation matrices start off at different rotations
            const float xRotation1 = 
                perlinNoise.Get(Vector2((float)mTime * frequency - 10.0f, (float)mTime * frequency - 20.0f))
                * maxRotationAmount + (float)M_PI;
            const float yRotation1 = 
                perlinNoise.Get(Vector2((float)mTime * frequency + 6.0f, (float)mTime * frequency - 8.0f))
                * maxRotationAmount + (float)M_PI;

            Matrix3 normalMapRotation0 =
                matrix::GetRotationX(xRotation0) *
                matrix::GetRotationY(yRotation0);
            Matrix3 normalMapRotation1 =
                matrix::GetRotationX(xRotation1) *
                matrix::GetRotationY(yRotation1);
            // ^^^ Normal map rotation calculation ^^^

           GL(glUniformMatrix3fv(10, 1, GL_FALSE, normalMapRotation0.GetPointerToData()));
           GL(glUniformMatrix3fv(11, 1, GL_FALSE, normalMapRotation1.GetPointerToData()));
        }));

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

    mTime += (double)mDeltaTime;
   
    mCamera.UpdatePosition(mDeltaTime);
    mTexturedMoon.Update(mDeltaTime);
    mAsteroidMoon.Update(mDeltaTime);
    mPlanet.Update(mDeltaTime);
}

void Game::Render() const
{
    BENCHMARK;
    mPostProcessor.Render("OceanEffect");
}

void Game::RenderWithPostProcessingEffect()
{
    BENCHMARK;
    mTexturedMoon.Render(mCamera, mProjectionMatrix);
    mAsteroidMoon.Render(mCamera, mProjectionMatrix);
    mPlanet.Render(mCamera, mProjectionMatrix);
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