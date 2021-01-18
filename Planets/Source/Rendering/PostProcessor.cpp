#include "PostProcessor.h"
#include "../Window/Window.h"
#include "GlMacro.h"

PostProcessor::PostProcessor(std::function<void()> renderingFunction)
	:
	mRenderingFunction(renderingFunction)
{
	InitializeTextures();
	InitializeFramebuffer();
}

PostProcessor::~PostProcessor()
{
	// Destructors should not throw exception, hence no GL macros
	glDeleteFramebuffers(1, &mFramebuffer);
	glDeleteTextures(1, &mTexture);
	glDeleteTextures(1, &mDepthTexture);
}

void PostProcessor::Render(const std::string& effect) const
{
	StartRenderingIntoTexture();
	mRenderingFunction();
	StopRenderingIntoTexture();
	// Render the texture, with the given effect: "effect"
	RenderTextureWithEffect(effect);
}

void PostProcessor::AddEffect(const std::string& effectName)
{
	// Make sure that we have not added this effect already
	assert(mNameToEffect.find(effectName) == mNameToEffect.end());

	mNameToEffect.insert({ effectName, Program(effectName) });
}

void PostProcessor::InitializeTextures()
{
	GL(glCreateTextures(GL_TEXTURE_2D, 1, &mTexture));
	GL(glTextureStorage2D(mTexture, 1, GL_RGBA8, Window::GetWidth(), Window::GetHeight()));

	GL(glCreateTextures(GL_TEXTURE_2D, 1, &mDepthTexture));
	GL(glTextureStorage2D(mDepthTexture, 1, GL_DEPTH_COMPONENT32F, Window::GetWidth(), Window::GetHeight()));
}

void PostProcessor::InitializeFramebuffer()
{
	GL(glCreateFramebuffers(1, &mFramebuffer));
	GL(glNamedFramebufferTexture(mFramebuffer, GL_COLOR_ATTACHMENT0, mTexture, 0));

	// We need to give the framebuffer a depth texture, in order for us to be able to use depth testing
	GL(glNamedFramebufferTexture(mFramebuffer, GL_DEPTH_ATTACHMENT, mDepthTexture, 0));

	// Make the rendering go into the texture, while we are using the framebuffer
	GL(glNamedFramebufferDrawBuffer(mFramebuffer, GL_COLOR_ATTACHMENT0));
}

void PostProcessor::StartRenderingIntoTexture() const
{
	GL(glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer));
	GL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
}

void PostProcessor::StopRenderingIntoTexture() const
{
	GL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

void PostProcessor::RenderTextureWithEffect(const std::string& effect) const
{
	// Make sure that the effect has been added
	assert(mNameToEffect.find(effect) != mNameToEffect.end());

	// Bind the effect
	mNameToEffect.at(effect).Bind();
	// Bind the texture
	GL(glBindTextureUnit(0, mTexture));

	GL(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
}
