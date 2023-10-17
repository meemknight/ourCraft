#include "rendering/skyBoxRenderer.h"

void SkyBoxRenderer::create()
{

	shader.loadShaderProgramFromFile(RESOURCES_PATH "skyBox.vert", RESOURCES_PATH "skyBox.frag");
	u_invView = shader.getUniform("u_invView");
	u_invProj = shader.getUniform("u_invProj");
	u_sunPos = shader.getUniform("u_sunPos");

}

void SkyBoxRenderer::render(Camera camera)
{

	shader.bind();
	glUniformMatrix4fv(u_invView, 1, GL_FALSE, &glm::inverse(camera.getViewMatrix())[0][0]);
	glUniformMatrix4fv(u_invProj, 1, GL_FALSE, &glm::inverse(camera.getProjectionMatrix())[0][0]);
	glUniform3fv(u_sunPos, 1, &sunPos[0]);

	glDepthMask(0);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glDepthMask(1);

}
