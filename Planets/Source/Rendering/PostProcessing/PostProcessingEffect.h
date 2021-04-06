#pragma once
#include "GL/glew.h"
#include "../GlMacro.h"
#include "../Program.h"

class PostProcessingEffect
{
public:
	PostProcessingEffect(Program program, std::function<void(GLuint, GLuint)> preparationFunction)
		:
		mProgram(std::move(program)),
		mPreparationFunction(preparationFunction)
	{}

	void Render(GLuint colourTexture, GLuint depthTexture) const
	{
		mProgram.Bind();
		mPreparationFunction(colourTexture, depthTexture);

		GL(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
	}

private:
	Program mProgram;
	std::function<void(GLuint, GLuint)> mPreparationFunction;
};