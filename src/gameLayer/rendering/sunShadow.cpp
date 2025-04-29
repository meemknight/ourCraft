#include <rendering/sunShadow.h>


#define GET_UNIFORM(s, n) n = s.getUniform(#n);


void SunShadow::init()
{

	int sizes[3] = {2048,2048,1024};

	for (int i = 0; i < 3; i++)
	{
		shadowMapCascades[i].create(false, true);
		shadowMapCascades[i].updateSize(sizes[i], sizes[i]);

		glBindTexture(GL_TEXTURE_2D, shadowMapCascades[i].depth);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		float border[] = {1, 1, 1, 1};
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border);

		bindlessShadowTextures[i] = glGetTextureHandleARB(shadowMapCascades[i].depth);
		glMakeTextureHandleResidentARB(bindlessShadowTextures[i]);
	}


	shadowTexturePreview.create(true, false);
	shadowTexturePreview.updateSize(sizes[0], sizes[0]);

	renderShadowIntoTextureShader.loadShaderProgramFromFile(RESOURCES_PATH "shaders/rendering/renderDepth.vert",
		RESOURCES_PATH "shaders/rendering/renderDepth.frag");

	GET_UNIFORM(renderShadowIntoTextureShader, u_depthTexture);
	GET_UNIFORM(renderShadowIntoTextureShader, u_far);
	GET_UNIFORM(renderShadowIntoTextureShader, u_close);

}

void SunShadow::update()
{

}

void SunShadow::renderShadowIntoTexture(Camera &camera)
{
	glBindFramebuffer(GL_FRAMEBUFFER, shadowTexturePreview.fbo);
	glClear(GL_COLOR_BUFFER_BIT);
	//glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDisable(GL_DEPTH_TEST);

	glViewport(0, 0, shadowTexturePreview.size.x, shadowTexturePreview.size.y);

	renderShadowIntoTextureShader.bind();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, shadowMapCascades[0].depth);
	glUniform1i(u_depthTexture, 0);
	//glUniform1f(u_far, 260.f);
	//glUniform1f(u_close, 1.f);

	glUniform1f(u_far, camera.farPlane);
	glUniform1f(u_close, camera.closePlane);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glEnable(GL_DEPTH_TEST);

}
