#version 330 core

layout(location = 0) out vec4 outColor;

noperspective in vec2 v_texCoords;

uniform sampler2D u_depthTexture;


void main()
{
	float sceneDepth = texture(u_depthTexture, v_texCoords).r; // Scene depth (0 to 1)
	
	if (sceneDepth < 1.0) 
	{
		outColor = vec4(0.0);
	}else
	{
		discard;
	}
}