#version 430 core
layout(location = 0) out vec4 color;

in vec2 v_uv;

layout(binding = 0) uniform sampler2D u_sunTexture;
layout (location = 1) out vec4 out_screenSpacePositions;
layout (location = 2) out ivec3 out_normals;
layout (location = 3) out vec3 out_bloom;

uniform float u_sunset;
uniform float u_colorMultiplier;

vec3 rgbTohsv(vec3 c)
{
	vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
	vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
	vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

	float d = q.x - min(q.w, q.y);
	float e = 1.0e-10;
	return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}
 

vec3 hsvTorgb(vec3 c)
{
	vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
	vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
	return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

void main()
{
	vec4 c = texture(u_sunTexture, v_uv).rgba;

	c.rgb = rgbTohsv(c.rgb);

	c.r -= u_sunset * 30/180.f;
	if(c.r < 0)c.r += 1;

	c.g += u_sunset * 2 / 180.f;
	if(c.g > 1)c.r = 1;

	c.rgb = hsvTorgb(c.rgb);

	color.rgba = c;
	color.rgb *= 1.5;

	color.rgb *= u_colorMultiplier;

	out_bloom.rgb = color.rgb * 0.014;
}