#version 430 core
#extension GL_ARB_bindless_texture: require

layout(location = 0) out vec4 color;

in vec2 v_uv;
in flat int v_id;

uniform sampler2D u_texture[6];



void main()
{

	color = texture(u_texture[v_id/6], v_uv).rgba;
	if(color.a <= 0){discard;}


}