#include "renderer.h"

#define GET_UNIFORM(s, n) n = s.getUniform(#n);


void Renderer::create()
{
	defaultShader.loadShaderProgramFromFile(RESOURCES_PATH "defaultShader.vert", RESOURCES_PATH "defaultShader.frag");
	defaultShader.bind();

	//u_viewProjection = defaultShader.getUniform("u_viewProjection");
	GET_UNIFORM(defaultShader, u_viewProjection);


}


#undef GET_UNIFORM