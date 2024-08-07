#version 330 core

layout (location = 0) out vec4 outColor;

uniform sampler2D u_depthTexture;
uniform float u_far;
uniform float u_close;

in vec2 v_fragCoord;

void main()
{

	vec2 uv = v_fragCoord;
	uv += vec2(1,1);
	uv /= 2.0;

	float c = texture(u_depthTexture, uv).r;
		
	//c = (2.0 * u_close) / (u_far + u_close - c * (u_far - u_close));

	outColor = vec4(c,c,c,1);
}