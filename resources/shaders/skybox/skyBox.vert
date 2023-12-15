#version 430

layout(location = 0) in vec3 aPos;

out vec3 v_viewDirection;

uniform mat4 u_viewProjection;

void main()
{
	v_viewDirection = normalize(aPos);
	vec4 pos = u_viewProjection * vec4(aPos, 1.0);
	gl_Position = pos.xyww;
}