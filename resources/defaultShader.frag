#version 430 core
#extension GL_ARB_bindless_texture: require

layout (location = 0) out vec4 out_color;

layout(binding = 0) uniform sampler2D u_texture;

in vec2 v_uv;
in float v_color;

in flat uvec2 v_textureSampler;

void main()
{
	//vec4 textureColor = texture(u_texture, v_uv);
	
	vec4 textureColor = texture(sampler2D(v_textureSampler), v_uv);
	
	if(textureColor.a < 0.1){discard;}
	out_color = vec4(textureColor.rgb*v_color,textureColor.a);
}