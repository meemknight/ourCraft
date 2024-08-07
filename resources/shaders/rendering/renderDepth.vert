#version 330 core

out vec2 v_fragCoord;

void main()
{
	vec4 positions[] = vec4[](vec4(-1,-1,0,1),vec4(1,1,0,1),vec4(-1,1,0,1),
						vec4(-1,-1,0,1), vec4(1,-1,0,1), vec4(1,1,0,1));
	gl_Position = positions[gl_VertexID];
	v_fragCoord = gl_Position.xy;
}