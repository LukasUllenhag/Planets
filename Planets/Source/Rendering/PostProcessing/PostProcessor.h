#pragma once
#include "PostProcessingEffect.h"

class PostProcessor
{
public:
	// "renderingFunction" is the function for which we are going to apply 
	// the post-processing effect to
	PostProcessor(std::function<void()> renderingFunction);
	~PostProcessor();

	// Renders "mRenderingFunction" with the post-processing effect: "effect"
	void Render(const std::string& effect) const;
	void AddEffect(const std::string& effectName, PostProcessingEffect effect);
private:
	// Initializes "mTexture" and "mDepthTexture"
	void InitializeTextures();
	void InitializeFramebuffer();

	// Subsequent rendering gets stored
	// inside the texture
	void StartRenderingIntoTexture() const;
	// Rendering goes into the back buffer
	void StopRenderingIntoTexture() const;
	// Renders the texture with the post-processing effect: "effect"
	void RenderTextureWithEffect(const std::string& effect) const;
private:
	// We will apply the post-processing effect to the rendering that
	// happens inside "mRenderingFunction"
	std::function<void()> mRenderingFunction = []{};
	std::unordered_map<std::string, PostProcessingEffect> mNameToEffect;

	// The framebuffer enables us to render into the texture
	GLuint mFramebuffer = 0;
	GLuint mTexture = 0;
	// Since we want to be able to do depth testing when we are rendering 
	// into the texture, we will need a depth texture
	GLuint mDepthTexture = 0;
};