#pragma once
#include <rendering/renderer.h>


struct SunShadow
{
	Renderer::FBO shadowMap;
	Renderer::FBO shadowTexturePreview;
	Shader renderShadowIntoTextureShader;
	
	GLint u_depthTexture;
	GLint u_far;
	GLint u_close;

	void init();

	void update();
	
	glm::mat4 lightSpaceMatrix{1.f};
	glm::ivec3 lightSpacePosition = {};

	void renderShadowIntoTexture(Camera &camera);
	
	//glm::mat4 getLightSpaceMatrix();

};