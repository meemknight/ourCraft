#version 430 core
#extension GL_ARB_bindless_texture: require

layout(location = 0) out vec4 color;
layout(location = 1) out vec4 position;

in vec2 v_uv;
in flat int v_id;
in vec3 v_vertexPosition;


uniform sampler2D u_texture[6];
uniform mat4 u_view;


void main()
{

	color = texture(u_texture[v_id/6], v_uv).rgba;
	if(color.a <= 0){discard;}
	position.xyz = (u_view * vec4(v_vertexPosition,1)).xyz;
}