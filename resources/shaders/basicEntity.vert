#version 430 core

layout(location = 0) in vec3 shape;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;
layout(location = 3) in int bone;


uniform mat4 u_viewProjection;
uniform mat4 u_modelMatrix;
uniform ivec3 u_cameraPositionInt;
uniform vec3 u_cameraPositionFloat;

uniform int u_bonesPerModel;

out vec2 v_uv;
out vec3 v_vertexPosition;
out vec3 v_normals;
flat out int test;

readonly restrict layout(std430) buffer u_skinningMatrix
{
	mat4 skinningMatrix[];
};


struct PerEntityData
{
	int entityPositionIntX;
	int entityPositionIntY;
	int entityPositionIntZ;

	float entityPositionFloatX;
	float entityPositionFloatY;
	float entityPositionFloatZ;

	uvec2 textureSampler;
};

flat out uvec2 v_textureSampler;

readonly restrict layout(std430) buffer u_perEntityData
{
	PerEntityData entityData[];
};

void main()
{
	ivec3 entityPosInt = ivec3(entityData[gl_InstanceID].entityPositionIntX, entityData[gl_InstanceID].entityPositionIntY, entityData[gl_InstanceID].entityPositionIntZ);
	vec3 entityPosFloat = vec3(entityData[gl_InstanceID].entityPositionFloatX, entityData[gl_InstanceID].entityPositionFloatY, entityData[gl_InstanceID].entityPositionFloatZ);

	v_textureSampler = entityData[gl_InstanceID].textureSampler;

	vec3 diffI = entityPosInt - u_cameraPositionInt;
	vec3 diffF = diffI - u_cameraPositionFloat + entityPosFloat;
	
	mat4 currentSkinningMatrix = skinningMatrix[bone + gl_InstanceID * u_bonesPerModel];
	test = bone;

	mat4 finalModel = u_modelMatrix * currentSkinningMatrix;

	vec4 posViewSemi = vec4(diffF + vec3(finalModel * vec4(shape, 1)), 1);

	vec4 posProjection = u_viewProjection * posViewSemi;
	
	gl_Position = posProjection;
	v_uv = uv;

	v_vertexPosition = posViewSemi.xyz;

	v_normals = normalize((transpose(inverse(finalModel)) * vec4(normal, 0.0)).xyz);
}