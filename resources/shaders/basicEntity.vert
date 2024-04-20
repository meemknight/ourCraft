#version 330 core

layout(location = 0) in vec3 shape;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;
layout(location = 3) in int bone;


uniform ivec3 u_entityPositionInt;
uniform vec3 u_entityPositionFloat;


uniform mat4 u_viewProjection;
uniform mat4 u_modelMatrix;
uniform ivec3 u_cameraPositionInt;
uniform vec3 u_cameraPositionFloat;

out vec2 v_uv;
out vec3 v_vertexPosition;
out vec3 v_normals;
flat out int test;

uniform mat4 u_skinningMatrix[10];

void main()
{

	vec3 diffI = u_entityPositionInt - u_cameraPositionInt;
	vec3 diffF = diffI - u_cameraPositionFloat + u_entityPositionFloat;
	
	mat4 skinningMatrix = u_skinningMatrix[bone];
	test = bone;

	mat4 finalModel = u_modelMatrix * skinningMatrix;

	vec4 posViewSemi = vec4(diffF + vec3(finalModel * vec4(shape, 1)), 1);

	vec4 posProjection = u_viewProjection * posViewSemi;
	
	gl_Position = posProjection;
	v_uv = uv;

	v_vertexPosition = posViewSemi.xyz;

	v_normals = normalize((transpose(inverse(finalModel)) * vec4(normal, 0.0)).xyz);
}