#version 430 core


layout(location = 0) in vec3 shape;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;


uniform mat4 u_viewProjection;

out vec2 v_uv;
out flat int v_id;
out flat vec3 v_normal;

void main()
{
	gl_Position = u_viewProjection * vec4(shape,1);
	v_uv = uv;
	v_id = gl_VertexID;
	v_normal = normal;
}