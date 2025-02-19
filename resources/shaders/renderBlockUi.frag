#version 430 core
#extension GL_ARB_bindless_texture: require

layout(location = 0) out vec4 color;

in vec2 v_uv;
in flat int v_id;

uniform sampler2D u_texture[6];
in flat vec3 v_normal;
uniform int u_useOneTexture;

void main()
{
	
	//color = vec4(1,0,0,1);
	if(u_useOneTexture != 0)
	{
		color = texture(u_texture[0], v_uv).rgba;
	}else
	{
		color = texture(u_texture[v_id/6], v_uv).rgba;
	}
	if(color.a <= 0){discard;}
	//color.a = sqrt(color.a);


	float light = 0.65;
	light += 0.35 * clamp(dot(v_normal, normalize(vec3(-0.5,0.8,0.2))),0,1);
	color.rgb *= light;

}