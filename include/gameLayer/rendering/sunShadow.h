#pragma once
#include <rendering/renderer.h>


struct SunShadow
{
	Renderer::FBO shadowMapCascades[3];
	Renderer::FBO shadowTexturePreview;
	Shader renderShadowIntoTextureShader;
	GLuint64 bindlessShadowTextures[3] = {};

	GLint u_depthTexture;
	GLint u_far;
	GLint u_close;

	void init();

	void update();
	
	glm::mat4 lightSpaceMatrixCascades[3]{glm::mat4(1.f)};
	glm::ivec3 lightSpacePositionCascades[3] = {};

	void renderShadowIntoTexture(Camera &camera);
	
	//glm::mat4 getLightSpaceMatrix();

};