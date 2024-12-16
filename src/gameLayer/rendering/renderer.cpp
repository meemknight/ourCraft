#include "rendering/renderer.h"
#include <ctime>
#include "blocks.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/mat3x3.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <blocksLoader.h>
#include <chunkSystem.h>
#include <gamePlayLogic.h>
#include <iostream>
#include <rendering/sunShadow.h>
#include <platformTools.h>
#include <gameplay/entityManagerClient.h>
#include <rendering/frustumCulling.h>
#include <rendering/model.h>
#include <glm/gtx/quaternion.hpp>
#include <lightSystem.h>
#include <rendering/renderSettings.h>

#include <imgui-docking/imgui/imgui.h>


#define GET_UNIFORM(s, n) n = s.getUniform(#n);
#define GET_UNIFORM2(s, n) s. n = s.shader.getUniform(#n);



float vertexDataOriginal[] = {
	//front
	0.5, 0.5, 0.5,
	-0.5, 0.5, 0.5,
	-0.5, -0.5, 0.5,
	-0.5, -0.5, 0.5,
	0.5, -0.5, 0.5,
	0.5, 0.5, 0.5,

	//back
	-0.5, -0.5, -0.5,
	-0.5, 0.5, -0.5,
	0.5, 0.5, -0.5,
	0.5, 0.5, -0.5,
	0.5, -0.5, -0.5,
	-0.5, -0.5, -0.5,

	//top
	-0.5, 0.5, -0.5,
	-0.5, 0.5, 0.5,
	0.5, 0.5, 0.5,
	0.5, 0.5, 0.5,
	0.5, 0.5, -0.5,
	-0.5, 0.5, -0.5,

	//bottom
	0.5, -0.5, 0.5,
	-0.5, -0.5, 0.5,
	-0.5, -0.5, -0.5,
	-0.5, -0.5, -0.5,
	0.5, -0.5, -0.5,
	0.5, -0.5, 0.5,

	//left
	-0.5, -0.5, 0.5,
	-0.5, 0.5, 0.5,
	-0.5, 0.5, -0.5,
	-0.5, 0.5, -0.5,
	-0.5, -0.5, -0.5,
	-0.5, -0.5, 0.5,

	//right
	0.5, 0.5, -0.5,
	0.5, 0.5, 0.5,
	0.5, -0.5, 0.5,
	0.5, -0.5, 0.5,
	0.5, -0.5, -0.5,
	0.5, 0.5, -0.5,
};

float vertexUVOriginal[] = {
	//front
	1, 1,
	0, 1,
	0, 0,
	0, 0,
	1, 0,
	1, 1,

	//back
	0, 0,
	0, 1,
	1, 1,
	1, 1,
	1, 0,
	0, 0,

	//top
	1, 1,
	0, 1,
	0, 0,
	0, 0,
	1, 0,
	1, 1,

	//bottom
	1, 1,
	0, 1,
	0, 0,
	0, 0,
	1, 0,
	1, 1,

	//left
	1, 0,
	1, 1,
	0, 1,
	0, 1,
	0, 0,
	1, 0,

	//right
	1, 1,
	0, 1,
	0, 0,
	0, 0,
	1, 0,
	1, 1,
};

float vertexData[] = {
	//front
	0.5, 0.5, 0.5,
	-0.5, 0.5, 0.5,
	-0.5, -0.5, 0.5,
	0.5, -0.5, 0.5,

	//back
	-0.5, -0.5, -0.5,
	-0.5, 0.5, -0.5,
	0.5, 0.5, -0.5,
	0.5, -0.5, -0.5,

	//top
	-0.5, 0.5, -0.5,
	-0.5, 0.5, 0.5,
	0.5, 0.5, 0.5,
	0.5, 0.5, -0.5,

	//bottom
	0.5, -0.5, 0.5,
	-0.5, -0.5, 0.5,
	-0.5, -0.5, -0.5,
	0.5, -0.5, -0.5,

	//left
	-0.5, -0.5, 0.5,
	-0.5, 0.5, 0.5,
	-0.5, 0.5, -0.5,
	-0.5, -0.5, -0.5,

	//right
	0.5, 0.5, -0.5,
	0.5, 0.5, 0.5,
	0.5, -0.5, 0.5,
	0.5, -0.5, -0.5,

	//grass
	0.5, 0.5, 0.5,
	-0.5, 0.5, -0.5,
	-0.5, -0.5, -0.5,
	0.5, -0.5, 0.5,

	-0.5, -0.5, -0.5,
	-0.5, 0.5, -0.5,
	0.5, 0.5, 0.5,
	0.5, -0.5, 0.5,

	-0.5, 0.5, 0.5,
	0.5, 0.5, -0.5,
	0.5, -0.5, -0.5,
	-0.5, -0.5, 0.5,

	-0.5, -0.5, 0.5,
	0.5, -0.5, -0.5,
	0.5, 0.5, -0.5,
	-0.5, 0.5, 0.5,
	
#pragma region leaves

	//moving leaves
	//front
	0.5, 0.5, 0.5,
	-0.5, 0.5, 0.5,
	-0.5, -0.5, 0.5,
	0.5, -0.5, 0.5,

	//back
	-0.5, -0.5, -0.5,
	-0.5, 0.5, -0.5,
	0.5, 0.5, -0.5,
	0.5, -0.5, -0.5,

	//top
	-0.5, 0.5, -0.5,
	-0.5, 0.5, 0.5,
	0.5, 0.5, 0.5,
	0.5, 0.5, -0.5,

	//bottom
	0.5, -0.5, 0.5,
	-0.5, -0.5, 0.5,
	-0.5, -0.5, -0.5,
	0.5, -0.5, -0.5,

	//left
	-0.5, -0.5, 0.5,
	-0.5, 0.5, 0.5,
	-0.5, 0.5, -0.5,
	-0.5, -0.5, -0.5,

	//right
	0.5, 0.5, -0.5,
	0.5, 0.5, 0.5,
	0.5, -0.5, 0.5,
	0.5, -0.5, -0.5,
#pragma endregion

#pragma region torch and water

	//torch
	//front
	0.0625, 0.125, 0.0625,
	-0.0625, 0.125, 0.0625,
	-0.0625, -0.5, 0.0625,
	0.0625, -0.5, 0.0625,

	//back
	-0.0625, -0.5, -0.0625,
	-0.0625, 0.125, -0.0625,
	0.0625, 0.125, -0.0625,
	0.0625, -0.5, -0.0625,

	//top
	-0.0625, 0.125, -0.0625,
	-0.0625, 0.125, 0.0625,
	0.0625, 0.125, 0.0625,
	0.0625, 0.125, -0.0625,

	//bottom
	0.0625, -0.5, 0.0625,
	-0.0625, -0.5, 0.0625,
	-0.0625, -0.5, -0.0625,
	0.0625, -0.5, -0.0625,

	//left
	-0.0625, -0.5, 0.0625,
	-0.0625, 0.125, 0.0625,
	-0.0625, 0.125, -0.0625,
	-0.0625, -0.5, -0.0625,

	//right
	0.0625, 0.125, -0.0625,
	0.0625, 0.125, 0.0625,
	0.0625, -0.5, 0.0625,
	0.0625, -0.5, -0.0625,

	//water
	//front
	0.5, 0.375, 0.5,
	-0.5, 0.375, 0.5,
	-0.5, -0.5, 0.5,
	0.5, -0.5, 0.5,

	//back
	-0.5, -0.5, -0.5,
	-0.5, 0.375, -0.5,
	0.5, 0.375, -0.5,
	0.5, -0.5, -0.5,

	//top
	-0.5, 0.375, -0.5,
	-0.5, 0.375, 0.5,
	0.5, 0.375, 0.5,
	0.5, 0.375, -0.5,

	//bottom
	0.5, -0.5, 0.5,
	-0.5, -0.5, 0.5,
	-0.5, -0.5, -0.5,
	0.5, -0.5, -0.5,

	//left
	-0.5, -0.5, 0.5,
	-0.5, 0.375, 0.5,
	-0.5, 0.375, -0.5,
	-0.5, -0.5, -0.5,

	//right
	0.5, 0.375, -0.5,
	0.5, 0.375, 0.5,
	0.5, -0.5, 0.5,
	0.5, -0.5, -0.5,

	//water taller down
	//front
	0.5, 0.375, 0.5,
	-0.5, 0.375, 0.5,
	-0.5, -0.625, 0.5,
	0.5, -0.625, 0.5,

	//back
	-0.5, -0.625, -0.5,
	-0.5, 0.375, -0.5,
	0.5, 0.375, -0.5,
	0.5, -0.625, -0.5,

	//left
	-0.5, -0.625, 0.5,
	-0.5, 0.375, 0.5,
	-0.5, 0.375, -0.5,
	-0.5, -0.625, -0.5,

	//right
	0.5, 0.375, -0.5,
	0.5, 0.375, 0.5,
	0.5, -0.625, 0.5,
	0.5, -0.625, -0.5,


	//water taller down and up
	//front
	0.5, 0.5, 0.5,
	-0.5, 0.5, 0.5,
	-0.5, -0.625, 0.5,
	0.5, -0.625, 0.5,

	//back
	-0.5, -0.625, -0.5,
	-0.5, 0.5, -0.5,
	0.5, 0.5, -0.5,
	0.5, -0.625, -0.5,

	//left
	-0.5, -0.625, 0.5,
	-0.5, 0.5, 0.5,
	-0.5, 0.5, -0.5,
	-0.5, -0.625, -0.5,

	//right
	0.5, 0.5, -0.5,
	0.5, 0.5, 0.5,
	0.5, -0.625, 0.5,
	0.5, -0.625, -0.5,
#pragma endregion

#pragma region stairs

	//half bottom
		//front
		0.5, 0.0, 0.5,
		-0.5, 0.0, 0.5,
		-0.5, -0.5, 0.5,
		0.5, -0.5, 0.5,

		//back
		-0.5, -0.5, -0.5,
		-0.5, 0.0, -0.5,
		0.5, 0.0, -0.5,
		0.5, -0.5, -0.5,

		//left
		-0.5, -0.5, 0.5,
		-0.5, 0.0, 0.5,
		-0.5, 0.0, -0.5,
		-0.5, -0.5, -0.5,

		//right
		0.5, 0.0, -0.5,
		0.5, 0.0, 0.5,
		0.5, -0.5, 0.5,
		0.5, -0.5, -0.5,

	//top corner piece
	//front left side
		-0.5, 0.0, 0,
		-0.5, 0.5, 0,
		-0.5, 0.5, -0.5,
		-0.5, 0.0, -0.5,

	//front right side
		0.5, 0.5, -0.5,
		0.5, 0.5, 0,
		0.5, 0.0, 0,
		0.5, 0.0, -0.5,

	//front left side
		-0.5, 0.0, 0.5,
		-0.5, 0.5, 0.5,
		-0.5, 0.5, 0,
		-0.5, 0.0, 0,

		//front right side
		0.5, 0.5, 0,
		0.5, 0.5, 0.5,
		0.5, 0.0, 0.5,
		0.5, 0.0, 0,

	//side
		0, 0.5, 0.5,
		-0.5, 0.5, 0.5,
		-0.5, 0, 0.5,
		0, 0, 0.5,

		-0.5, 0, -0.5,
		-0.5, 0.5, -0.5,
		0, 0.5, -0.5,
		0, 0, -0.5,

	//side
		0.5, 0.5, 0.5,
		0, 0.5, 0.5,
		0, 0, 0.5,
		0.5, 0, 0.5,

		0, 0, -0.5,
		0, 0.5, -0.5,
		0.5, 0.5, -0.5,
		0.5, 0, -0.5,

	//top half
		//0
		-0.5, 0.5, -0.5,
		-0.5, 0.5, 0,
		0.5, 0.5, 0,
		0.5, 0.5, -0.5,

		//2
		-0.5, 0.5, 0,
		-0.5, 0.5, 0.5,
		0.5, 0.5, 0.5,
		0.5, 0.5, 0,

		//1
		-0.5, 0.5, -0.5,
		-0.5, 0.5, 0.5,
		0.0, 0.5, 0.5,
		0.0, 0.5, -0.5,


		//3
		0, 0.5, -0.5,
		0, 0.5, 0.5,
		0.5, 0.5, 0.5,
		0.5, 0.5, -0.5,

	//top half but bottom part
	//0
		-0.5, 0, -0.5,
		-0.5, 0, 0,
		0.5, 0, 0,
		0.5, 0, -0.5,

		//2
		-0.5, 0, 0,
		-0.5, 0, 0.5,
		0.5, 0, 0.5,
		0.5, 0, 0,

		//1
		-0.5, 0, -0.5,
		-0.5, 0, 0.5,
		0.0, 0, 0.5,
		0.0, 0, -0.5,


		//3
		0, 0, -0.5,
		0, 0, 0.5,
		0.5, 0, 0.5,
		0.5, 0, -0.5,

	//frontal middle top piece
		//front
		0.5, 0.5, 0,
		-0.5, 0.5, 0,
		-0.5, 0, 0,
		0.5, 0, 0,

		//back
		-0.5, 0, 0,
		-0.5, 0.5, 0,
		0.5, 0.5, 0,
		0.5, 0, 0,

		//right
		0, 0.5, -0.5,
		0, 0.5, 0.5,
		0, 0, 0.5,
		0, 0, -0.5,

		//left
		0, 0, 0.5,
		0, 0.5, 0.5,
		0, 0.5, -0.5,
		0, 0, -0.5,
#pragma endregion

#pragma region slabs

	//bottom slabs!!!
	//top
	-0.5, 0, -0.5,
	-0.5, 0, 0.5,
	0.5, 0, 0.5,
	0.5, 0, -0.5,

	//half top
	//front
			0.5, 0.5, 0.5,
			-0.5, 0.5, 0.5,
			-0.5, 0.0, 0.5,
			0.5, 0.0, 0.5,

			//back
			-0.5, 0.0, -0.5,
			-0.5, 0.5, -0.5,
			0.5, 0.5, -0.5,
			0.5, 0.0, -0.5,

			//left
			-0.5, 0.0, 0.5,
			-0.5, 0.5, 0.5,
			-0.5, 0.5, -0.5,
			-0.5, 0.0, -0.5,

			//right
			0.5, 0.5, -0.5,
			0.5, 0.5, 0.5,
			0.5, 0.0, 0.5,
			0.5, 0.0, -0.5,

		//bottom moddle part
		0.5, 0.0, 0.5,
		-0.5, 0.0, 0.5,
		-0.5, 0.0, -0.5,
		0.5, 0.0, -0.5,
#pragma endregion


	//walls inner part

		//front
		0.5, 0.5, 0,
		-0.5, 0.5, 0,
		-0.5, -0.5, 0,
		0.5, -0.5, 0,

		//back
		-0.5, -0.5, -0,
		-0.5, 0.5, -0,
		0.5, 0.5, -0,
		0.5, -0.5, -0,

		//left
		-0, -0.5, 0.5,
		-0, 0.5, 0.5,
		-0, 0.5, -0.5,
		-0, -0.5, -0.5,

		//right
		0, 0.5, -0.5,
		0, 0.5, 0.5,
		0, -0.5, 0.5,
		0, -0.5, -0.5,


	//walls bottom half
	//0
	0.5, -0.5, -0.5,
	0.5, -0.5, 0,
	-0.5, -0.5, 0,
	-0.5, -0.5, -0.5,

	//2
	0.5, -0.5, 0,
	0.5, -0.5, 0.5,
	-0.5, -0.5, 0.5,
	-0.5, -0.5, 0,

	//1
	0.0, -0.5, -0.5,
	0.0, -0.5, 0.5,
	-0.5, -0.5, 0.5,
	-0.5, -0.5, -0.5,


	//3
	0.5, -0.5, -0.5,
	0.5, -0.5, 0.5,
	0, -0.5, 0.5,
	0, -0.5, -0.5,

	//walls side
		//front left
		0.0, 0.5, 0.5,
		-0.5, 0.5, 0.5,
		-0.5, -0.5, 0.5,
		0.0, -0.5, 0.5,


		//front right
		0.5, 0.5, 0.5,
		0.0, 0.5, 0.5,
		0.0, -0.5, 0.5,
		0.5, -0.5, 0.5,


		//back left
		-0.5, -0.5, -0.5,
		-0.5, 0.5, -0.5,
		0.0, 0.5, -0.5,
		0.0, -0.5, -0.5,
				
		//back right
		0, -0.5, -0.5,
		0, 0.5, -0.5,
		0.5, 0.5, -0.5,
		0.5, -0.5, -0.5,


		//left front
		-0.5, -0.5, 0.0,
		-0.5, 0.5, 0.0,
		-0.5, 0.5, -0.5,
		-0.5, -0.5, -0.5,

		//left back
		-0.5, -0.5, 0.5,
		-0.5, 0.5, 0.5,
		-0.5, 0.5, 0.0,
		-0.5, -0.5, 0.0,

		//right front
		0.5, 0.5, -0.5,
		0.5, 0.5, 0.0,
		0.5, -0.5, 0.0,
		0.5, -0.5, -0.5,

		//right back
		0.5, 0.5, 0.0,
		0.5, 0.5, 0.5,
		0.5, -0.5, 0.5,
		0.5, -0.5, 0.0,

};

float vertexUV[] = {
	//front
	1, 1,
	0, 1,
	0, 0,
	1, 0,

	//back
	0, 0,
	0, 1,
	1, 1,
	1, 0,

	//top
	1, 1,
	0, 1,
	0, 0,
	1, 0,

	//bottom
	1, 1,
	0, 1,
	0, 0,
	1, 0,

	//left
	1, 0,
	1, 1,
	0, 1,
	0, 0,

	//right
	1, 1,
	0, 1,
	0, 0,
	1, 0,

#pragma region other

	//grass
	//front
	1, 1,
	0, 1,
	0, 0,
	1, 0,

	0, 0,
	0, 1,
	1, 1,
	1, 0,

	1, 1,
	0, 1,
	0, 0,
	1, 0,

	1, 0,
	0, 0,
	0, 1,
	1, 1,


	//leaves////////////////////////
	//front
	1, 1,
	0, 1,
	0, 0,
	1, 0,

	//back
	0, 0,
	0, 1,
	1, 1,
	1, 0,

	//top
	1, 1,
	0, 1,
	0, 0,
	1, 0,

	//bottom
	1, 1,
	0, 1,
	0, 0,
	1, 0,

	//left
	1, 0,
	1, 1,
	0, 1,
	0, 0,

	//right
	1, 1,
	0, 1,
	0, 0,
	1, 0,


	//torches ///////
	//front
	0.5625, 0.625,
	0.4375, 0.625,
	0.4375, 0,
	0.5625, 0,

	//back
	0.4375, 0,
	0.4375, 0.625,
	0.5625, 0.625,
	0.5625, 0,

	//top
	0.5625, 0.625,
	0.4375, 0.625,
	0.4375, 0.5,
	0.5625, 0.5,

	//bottom
	0.5625, 0.125,
	0.4375, 0.125,
	0.4375, 0,
	0.5625, 0,

	//left
	0.5625, 0,
	0.5625, 0.625,
	0.4375, 0.625,
	0.4375, 0,

	//right
	0.5625, 0.625,
	0.4375, 0.625,
	0.4375, 0,
	0.5625, 0,

	//water
	//front
	1, 0.875,
	0, 0.875,
	0, 0,
	1, 0,

	//back
	0, 0,
	0, 0.875,
	1, 0.875,
	1, 0,

	//top
	1, 1,
	0, 1,
	0, 0,
	1, 0,

	//bottom
	1, 1,
	0, 1,
	0, 0,
	1, 0,

	//left
	1, 0,
	1, 0.875,
	0, 0.875,
	0, 0,

	//right
	1, 0.875,
	0, 0.875,
	0, 0,
	1, 0,


	//water taller down,
	//front
	1, 0.875,
	0, 0.875,
	0, 0,
	1, 0,

	//back
	0, 0,
	0, 0.875,
	1, 0.875,
	1, 0,
	//left
	1, 0,
	1, 0.875,
	0, 0.875,
	0, 0,

	//right
	1, 0.875,
	0, 0.875,
	0, 0,
	1, 0,


	//
	//front
	1, 1,
	0, 1,
	0, 0,
	1, 0,

	//back
	0, 0,
	0, 1,
	1, 1,
	1, 0,

	//left
	1, 0,
	1, 1,
	0, 1,
	0, 0,

	//right
	1, 1,
	0, 1,
	0, 0,
	1, 0,
#pragma endregion

#pragma region stairs


	//half botom
		//front
		1, 0.5,
		0, 0.5,
		0, 0,
		1, 0,

		//back
		0, 0,
		0, 0.5,
		1, 0.5,
		1, 0,

		//left
		1, 0,
		1, 0.5,
		0, 0.5,
		0, 0,

		//right
		1, 0.5,
		0, 0.5,
		0, 0,
		1, 0,

	//top corner piece
	//front left side
		0.5, 0.5,
		0.5, 1,
		0, 1,
		0, 0.5,

	//front right side
		0.5, 1,
		0, 1,
		0, 0.5,
		0.5, 0.5,

		//top corner piece
	//front left side
	1, 0.5,
	1, 1,
	0.5, 1,
	0.5, 0.5,
	
	//front right side
	1, 1,
	0.5, 1,
	0.5, 0.5,
	1, 0.5,

	//side
	0.5, 1,
	0, 1,
	0, 0.5,
	0.5, 0.5,

	//side
	0, 0.5,
	0, 1,
	0.5, 1,
	0.5, 0.5,

	//side
	1, 1,
	0.5, 1,
	0.5, 0.5,
	1, 0.5,

	//side
	0.5, 0.5,
	0.5, 1,
	1, 1,
	1, 0.5,

	//top half stairt
	1, 1,
	0.5, 1,
	0.5, 0,
	1, 0,

	0.5, 1,
	0, 1,
	0, 0,
	0.5, 0,

	1, 0.5,
	0, 0.5,
	0, 0,
	1, 0,

	1, 1,
	0, 1,
	0, 0.5,
	1, 0.5,


	//top half stairt but bottom part
		1, 1,
		0.5, 1,
		0.5, 0,
		1, 0,

		0.5, 1,
		0, 1,
		0, 0,
		0.5, 0,

		1, 0.5,
		0, 0.5,
		0, 0,
		1, 0,

		1, 1,
		0, 1,
		0, 0.5,
		1, 0.5,

	//frontal middle top piece
		//front
		1, 1,
		0, 1,
		0, 0.5,
		1, 0.5,

		//back
		0, 0.5,
		0, 1,
		1, 1,
		1, 0.5,

		//right
		1, 1,
		0.5, 1,
		0.5, 0.5,
		1, 0.5,

		//left
		0.5, 0.5,
		0.5, 1,
		0, 1,
		0, 0.5,
#pragma endregion


	//bottom slabs!!!
	//top
	1, 1,
	0, 1,
	0, 0,
	1, 0,

	//half top
	//front
		1, 1.0,
		0, 1.0,
		0, 0.5,
		1, 0.5,

		//back
		0, 0.5,
		0, 1.0,
		1, 1.0,
		1, 0.5,

		//left
		1, 0.5,
		1, 1.0,
		0, 1.0,
		0, 0.5,

		//right
		1, 1.0,
		0, 1.0,
		0, 0.5,
		1, 0.5,

	//bottom
	1, 1,
	0, 1,
	0, 0,
	1, 0,

	
	//walls inner part
		//front
		1, 1,
		0, 1,
		0, 0,
		1, 0,

		//back
		0, 0,
		0, 1,
		1, 1,
		1, 0,

		//left
		1, 0,
		1, 1,
		0, 1,
		0, 0,

		//right
		1, 1,
		0, 1,
		0, 0,
		1, 0,

	//bottom half walls

	1, 0,
	0.5, 0,
	0.5, 1,
	1, 1,

	0.5, 0,
	0, 0,
	0, 1,
	0.5, 1,

	1, 0,
	0, 0,
	0, 0.5,
	1, 0.5,

	1, 0.5,
	0, 0.5,
	0, 1,
	1, 1,


	//walls side
	//front left
	0.5, 1,
	0, 1,
	0, 0,
	0.5, 0,

	//front right
	1, 1,
	0.5, 1,
	0.5, 0,
	1, 0,

	//back left
	0, 0,
	0, 1,
	0.5, 1,
	0.5, 0,

	//back right
	0.5, 0,
	0.5, 1,
	1, 1,
	1, 0,

	//left front
	0.5, 0,
	0.5, 1,
	0, 1,
	0, 0,

	//left back
	1, 0,
	1, 1,
	0.5, 1,
	0.5, 0,


	//right front
	0.5, 1,
	0, 1,
	0, 0,
	0.5, 0,


	//right back
	1, 1,
	0.5, 1,
	0.5, 0,
	1, 0,
};


constexpr float uv = 1;
constexpr float cubeEntityData[] = {
	-0.5f, +0.5f, +0.5f, // 0
	+0.0f, +1.0f, +0.0f, // Normal
	0, 0,				 //uv

	+0.5f, +0.5f, +0.5f, // 1
	+0.0f, +1.0f, +0.0f, // Normal
	1 * uv, 0,				 //uv

	+0.5f, +0.5f, -0.5f, // 2
	+0.0f, +1.0f, +0.0f, // Normal
	1 * uv, 1 * uv,				 //uv

	-0.5f, +0.5f, -0.5f, // 3
	+0.0f, +1.0f, +0.0f, // Normal
	0, 1 * uv,				 //uv

	-0.5f, +0.5f, -0.5f, // 4
	 0.0f, +0.0f, -1.0f, // Normal
	 0, 1 * uv,				 //uv

	 +0.5f, +0.5f, -0.5f, // 5
	  0.0f, +0.0f, -1.0f, // Normal
	  1 * uv, 1 * uv,				 //uv

	   +0.5f, -0.5f, -0.5f, // 6
	   0.0f, +0.0f, -1.0f, // Normal
	   1 * uv, 0,				 //uv

	  -0.5f, -0.5f, -0.5f, // 7
	   0.0f, +0.0f, -1.0f, // Normal
	   0, 0,				 //uv

	   +0.5f, +0.5f, -0.5f, // 8
	   +1.0f, +0.0f, +0.0f, // Normal
	   1 * uv, 0,				 //uv

	   +0.5f, +0.5f, +0.5f, // 9
	   +1.0f, +0.0f, +0.0f, // Normal
	   1 * uv, 1 * uv,				 //uv

	   +0.5f, -0.5f, +0.5f, // 10
	   +1.0f, +0.0f, +0.0f, // Normal
	   0, 1 * uv,				 //uv

	   +0.5f, -0.5f, -0.5f, // 11
	   +1.0f, +0.0f, +0.0f, // Normal
	   0, 0,				 //uv

	   -0.5f, +0.5f, +0.5f, // 12
	   -1.0f, +0.0f, +0.0f, // Normal
	   1 * uv, 1 * uv,				 //uv

	   -0.5f, +0.5f, -0.5f, // 13
	   -1.0f, +0.0f, +0.0f, // Normal
	   1 * uv, 0,				 //uv

	   -0.5f, -0.5f, -0.5f, // 14
	   -1.0f, +0.0f, +0.0f, // Normal
	   0, 0,				 //uv

	   -0.5f, -0.5f, +0.5f, // 15
	   -1.0f, +0.0f, +0.0f, // Normal
	   0, 1 * uv,				 //uv

	   +0.5f, +0.5f, +0.5f, // 16
	   +0.0f, +0.0f, +1.0f, // Normal
	   1 * uv, 1 * uv,				 //uv

	   -0.5f, +0.5f, +0.5f, // 17
	   +0.0f, +0.0f, +1.0f, // Normal
	   0, 1 * uv,				 //uv

	   -0.5f, -0.5f, +0.5f, // 18
	   +0.0f, +0.0f, +1.0f, // Normal
	   0, 0,				 //uv

	   +0.5f, -0.5f, +0.5f, // 19
	   +0.0f, +0.0f, +1.0f, // Normal
	   1 * uv, 0,				 //uv


	   +0.5f, -0.5f, -0.5f, // 20
	   +0.0f, -1.0f, +0.0f, // Normal
	   1 * uv, 0,				 //uv

	   -0.5f, -0.5f, -0.5f, // 21
	   +0.0f, -1.0f, +0.0f, // Normal
	   0, 0,				 //uv

	   -0.5f, -0.5f, +0.5f, // 22
	   +0.0f, -1.0f, +0.0f, // Normal
	   0, 1 * uv,				 //uv

	   +0.5f, -0.5f, +0.5f, // 23
	   +0.0f, -1.0f, +0.0f, // Normal
	   1 * uv, 1 * uv,				 //uv

};

unsigned int cubeEntityIndices[] = {
16, 17, 18, 16, 18, 19, // Front
4,   5,  6,  4,  6,  7, // Back
0,   1,  2,  0,  2,  3, // Top
20, 22, 21, 20, 23, 22, // Bottom
12, 13, 14, 12, 14, 15, // Left
8,   9, 10,  8, 10, 11, // Right
};


void Renderer::recreateBlocksTexturesBuffer(BlocksLoader &blocksLoader)
{

	//glDeleteBuffers(1, &textureSamplerersBuffer);
	//if (!textureSamplerersBuffer)
	//{
	//	glGenBuffers(1, &textureSamplerersBuffer);
	//
	//	glBindBuffer(GL_SHADER_STORAGE_BUFFER, textureSamplerersBuffer);
	//	glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(GLuint64) * blocksLoader.gpuIds.size(), blocksLoader.gpuIds.data(), 0);
	//	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, textureSamplerersBuffer);
	//}
	//else
	//{
	//	glBindBuffer(GL_SHADER_STORAGE_BUFFER, textureSamplerersBuffer);
	//	void *ptr = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY);
	//	if (ptr)
	//	{
	//		memcpy(ptr, blocksLoader.gpuIds.data(), sizeof(GLuint64) * blocksLoader.gpuIds.size());
	//		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	//	}
	//	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, textureSamplerersBuffer);
	//
	//	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	//}


	if (!textureSamplerersBuffer)
	glGenBuffers(1, &textureSamplerersBuffer);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, textureSamplerersBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLuint64) * blocksLoader.gpuIds.size(), blocksLoader.gpuIds.data(), GL_STATIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, textureSamplerersBuffer);


	defaultShader.u_textureSamplerers = getStorageBlockIndex(defaultShader.shader.id, "u_textureSamplerers");
	glShaderStorageBlockBinding(defaultShader.shader.id, defaultShader.u_textureSamplerers, 3);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void Renderer::create()
{

	for (int i = 0; i < sizeof(sunFlareQueries) / sizeof(sunFlareQueries[0]); i++)
	{
		sunFlareQueries[i].create();
	}

	fboCoppy.create(GL_R11F_G11F_B10F, true);
	//filteredBloomColor.create(GL_R11F_G11F_B10F, false);

	fboMain.create(GL_R11F_G11F_B10F, true, GL_RGB16F, GL_RGB16UI, GL_R11F_G11F_B10F);
	glBindTexture(GL_TEXTURE_2D, fboMain.secondaryColor);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	fboLastFrame.create(true, false);
	fboLastFramePositions.create(GL_RGB16F, false);
	fboHBAO.create(GL_RED, false);
	fboSkyBox.create(GL_R11F_G11F_B10F, false);


	glGenBuffers(1, &automatixExposureReadBUffer);
	glBindBuffer(GL_PIXEL_PACK_BUFFER, automatixExposureReadBUffer);
	glBufferData(GL_PIXEL_PACK_BUFFER, 4 * sizeof(GLfloat), NULL, GL_STREAM_READ);
	glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);


	{

		glGenFramebuffers(1, &filterFbo);
		glBindFramebuffer(GL_FRAMEBUFFER, filterFbo);

		glGenFramebuffers(2, blurFbo);
		glGenTextures(2, bluredColorBuffer);

		//bluredBloomColor[0].create(GL_R11F_G11F_B10F, false);
		//bluredBloomColor[1].create(GL_R11F_G11F_B10F, false);

		for (int i = 0; i < 2; i++)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, blurFbo[i]);
			float borderColor[] = {0.0f, 0.0f, 0.0f, 0.0f};

			glBindTexture(GL_TEXTURE_2D, bluredColorBuffer[i]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_R11F_G11F_B10F, 1, 1, 0, GL_RGB, GL_FLOAT, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
			glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bluredColorBuffer[i], 0);
		}


	}


	{
		glGenVertexArrays(1, &vaoQuad);
		glBindVertexArray(vaoQuad);
		glGenBuffers(1, &vertexBufferQuad);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBufferQuad);

		//how to use
		//glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		float quadVertices[] = {
			// positions        // texture Coords
			-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
			 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		};

		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));

		glBindVertexArray(0);
	};


	glGenBuffers(1, &defaultShader.shadingSettingsBuffer);
	glBindBuffer(GL_UNIFORM_BUFFER, defaultShader.shadingSettingsBuffer);
	

	reloadShaders();

	
	
	glGenBuffers(1, &vertexDataBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, vertexDataBuffer);
	glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(vertexData), vertexData, 0);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, vertexDataBuffer);

	
	glGenBuffers(1, &vertexUVBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, vertexUVBuffer);
	glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(vertexUV), vertexUV, 0);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, vertexUVBuffer);


	

	
	glGenBuffers(1, &lightBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	
	//todo remove?
	//not in use anymore?
	glCreateBuffers(1, &vertexBuffer);
	//glNamedBufferData(vertexBuffer, sizeof(data), data, GL_DYNAMIC_DRAW);

	glCreateVertexArrays(1, &vao);
	glBindVertexArray(vao);
		
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);

		glEnableVertexAttribArray(0);
		glVertexAttribIPointer(0, 1, GL_SHORT, 4 * sizeof(int), 0);
		glVertexAttribDivisor(0, 1);

		glEnableVertexAttribArray(1);
		glVertexAttribIPointer(1, 1, GL_SHORT, 4 * sizeof(int), (void *)(1 * sizeof(short)));
		glVertexAttribDivisor(1, 1);

		glEnableVertexAttribArray(2);
		glVertexAttribIPointer(2, 3, GL_INT, 4 * sizeof(int), (void *)(1 * sizeof(int)));
		glVertexAttribDivisor(2, 1);
		
		//glEnableVertexAttribArray(3);
		//glVertexAttribIPointer(3, 1, GL_INT, 5 * sizeof(int), (void *)(4 * sizeof(int)));
		//glVertexAttribDivisor(3, 1);
		//
		//glEnableVertexAttribArray(4);
		//glVertexAttribIPointer(4, 1, GL_INT, 6 * sizeof(int), (void *)(5 * sizeof(int)));
		//glVertexAttribDivisor(4, 1);

	glBindVertexArray(0);

	glGenBuffers(1, &drawCommandsOpaqueBuffer);
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, drawCommandsOpaqueBuffer);
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);



#pragma region basic entity renderer

	//GLuint vaoCube = 0;
	//GLuint vertexBufferCube = 0;
	//GLuint indexBufferCube = 0;

	glCreateVertexArrays(1, &entityRenderer.blockEntityshader.vaoCube);
	glBindVertexArray(entityRenderer.blockEntityshader.vaoCube);

	glGenBuffers(1, &entityRenderer.blockEntityshader.vertexBufferCube);
	glBindBuffer(GL_ARRAY_BUFFER, entityRenderer.blockEntityshader.vertexBufferCube);

	//glBufferData(GL_ARRAY_BUFFER, sizeof(cubeEntityData), cubeEntityData, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), 0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));

	std::vector<float> data;
	for (int face = 0; face < 6; face++)
	{
		for (int i = 0; i < 6; i++)
		{
			
			data.push_back(vertexDataOriginal[i * 3 + face * 3 * 6 + 0]);
			data.push_back(vertexDataOriginal[i * 3 + face * 3 * 6 + 1]);
			data.push_back(vertexDataOriginal[i * 3 + face * 3 * 6 + 2]);

			//todo normal
			data.push_back(0);
			data.push_back(1);
			data.push_back(0);

			data.push_back(vertexUVOriginal[i * 2 + face * 2 * 6 + 0]);
			data.push_back(vertexUVOriginal[i * 2 + face * 2 * 6 + 1]);

		}
	}

	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * data.size(), data.data(), GL_STATIC_DRAW);



	//glGenBuffers(1, &entityRenderer.basicEntityshader.indexBufferCube);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, entityRenderer.basicEntityshader.indexBufferCube);
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeEntityIndices), cubeEntityIndices, GL_STATIC_DRAW);

	glBindVertexArray(0);



	glGenBuffers(1, &skinningMatrixSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, skinningMatrixSSBO);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, skinningMatrixSSBO);
	

	glGenBuffers(1, &perEntityDataSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, perEntityDataSSBO);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, perEntityDataSSBO);

#pragma endregion


#pragma region decals
	{


		unsigned char winding[4] = {0,1,2,3};

		glGenBuffers(1, &decalShader.geometry);
		glGenBuffers(1, &decalShader.index);
		glGenVertexArrays(1, &decalShader.vao);
		glBindVertexArray(decalShader.vao);
		glBindBuffer(GL_ARRAY_BUFFER, decalShader.geometry);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, decalShader.index);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(winding), winding, GL_STATIC_DRAW);
		setupVertexAttributes();
		

		glBindVertexArray(0);
	}


#pragma endregion



}

void Renderer::reloadShaders()
{

	defaultShader.shader.clear();

	defaultShader.shader.loadShaderProgramFromFile(RESOURCES_PATH "shaders/rendering/defaultShader.vert",
		RESOURCES_PATH "shaders/rendering/defaultShader.frag");
	defaultShader.shader.bind();

	GET_UNIFORM2(defaultShader, u_viewProjection);
	GET_UNIFORM2(defaultShader, u_typesCount);
	GET_UNIFORM2(defaultShader, u_positionInt);
	GET_UNIFORM2(defaultShader, u_positionFloat);
	GET_UNIFORM2(defaultShader, u_texture);
	GET_UNIFORM2(defaultShader, u_time);
	GET_UNIFORM2(defaultShader, u_showLightLevels);
	GET_UNIFORM2(defaultShader, u_skyLightIntensity);
	GET_UNIFORM2(defaultShader, u_lightsCount);
	GET_UNIFORM2(defaultShader, u_pointPosF);
	GET_UNIFORM2(defaultShader, u_pointPosI);
	GET_UNIFORM2(defaultShader, u_sunDirection);
	GET_UNIFORM2(defaultShader, u_metallic);
	GET_UNIFORM2(defaultShader, u_roughness);
	GET_UNIFORM2(defaultShader, u_underWater);
	GET_UNIFORM2(defaultShader, u_sunLightColor);
	GET_UNIFORM2(defaultShader, u_ambientColor);
	GET_UNIFORM2(defaultShader, u_waterColor);
	GET_UNIFORM2(defaultShader, u_depthPeelwaterPass);
	GET_UNIFORM2(defaultShader, u_depthTexture);
	GET_UNIFORM2(defaultShader, u_hasPeelInformation);
	GET_UNIFORM2(defaultShader, u_PeelTexture);
	GET_UNIFORM2(defaultShader, u_dudv);
	GET_UNIFORM2(defaultShader, u_dudvNormal);
	GET_UNIFORM2(defaultShader, u_waterMove);
	GET_UNIFORM2(defaultShader, u_near);
	GET_UNIFORM2(defaultShader, u_far);
	GET_UNIFORM2(defaultShader, u_caustics);
	GET_UNIFORM2(defaultShader, u_inverseProjMat);
	GET_UNIFORM2(defaultShader, u_lightSpaceMatrix);
	GET_UNIFORM2(defaultShader, u_lightPos);
	GET_UNIFORM2(defaultShader, u_sunShadowTexture);
	GET_UNIFORM2(defaultShader, u_timeGrass);
	GET_UNIFORM2(defaultShader, u_writeScreenSpacePositions);
	GET_UNIFORM2(defaultShader, u_lastFrameColor);
	GET_UNIFORM2(defaultShader, u_lastFramePositionViewSpace);
	GET_UNIFORM2(defaultShader, u_cameraProjection);
	GET_UNIFORM2(defaultShader, u_inverseView);
	GET_UNIFORM2(defaultShader, u_view);
	GET_UNIFORM2(defaultShader, u_brdf);
	GET_UNIFORM2(defaultShader, u_inverseViewProjMat);
	GET_UNIFORM2(defaultShader, u_lastViewProj);
	GET_UNIFORM2(defaultShader, u_skyTexture);
	GET_UNIFORM2(defaultShader, u_ao);
	GET_UNIFORM2(defaultShader, u_baseAmbientExtra);
	

	defaultShader.u_shadingSettings
		= glGetUniformBlockIndex(defaultShader.shader.id, "ShadingSettings");
	glBindBufferBase(GL_UNIFORM_BUFFER, 
		defaultShader.u_shadingSettings, defaultShader.shadingSettingsBuffer);

	//todo enum for all samplers
	defaultShader.u_vertexData = getStorageBlockIndex(defaultShader.shader.id, "u_vertexData");
	glShaderStorageBlockBinding(defaultShader.shader.id, defaultShader.u_vertexData, 1);

	defaultShader.u_vertexUV = getStorageBlockIndex(defaultShader.shader.id, "u_vertexUV");
	glShaderStorageBlockBinding(defaultShader.shader.id, defaultShader.u_vertexUV, 2);

	defaultShader.u_textureSamplerers = getStorageBlockIndex(defaultShader.shader.id, "u_textureSamplerers");
	glShaderStorageBlockBinding(defaultShader.shader.id, defaultShader.u_textureSamplerers, 3);

	defaultShader.u_lights = getStorageBlockIndex(defaultShader.shader.id, "u_lights");
	glShaderStorageBlockBinding(defaultShader.shader.id, defaultShader.u_lights, 4);

#pragma region bloom

	filterBloomDataShader.shader.clear();
	filterBloomDataShader.shader.loadShaderProgramFromFile(RESOURCES_PATH "shaders/postProcess/drawQuads.vert",
		RESOURCES_PATH "shaders/postProcess/filterBloomData.frag");
	filterBloomDataShader.shader.bind();
	GET_UNIFORM2(filterBloomDataShader, u_exposure);
	GET_UNIFORM2(filterBloomDataShader, u_tresshold);
	GET_UNIFORM2(filterBloomDataShader, u_multiplier);


	applyBloomDataShader.shader.clear();
	applyBloomDataShader.shader.loadShaderProgramFromFile(RESOURCES_PATH "shaders/postProcess/drawQuads.vert",
		RESOURCES_PATH "shaders/postProcess/applyBloomData.frag");
	applyBloomDataShader.shader.bind();
	GET_UNIFORM2(applyBloomDataShader, u_waterDropsPower);




	filterDownShader.shader.clear();
	filterDownShader.shader.loadShaderProgramFromFile(RESOURCES_PATH "shaders/postProcess/drawQuads.vert",
		RESOURCES_PATH "shaders/postProcess/filterDown.frag");
	GET_UNIFORM2(filterDownShader, u_texture);
	GET_UNIFORM2(filterDownShader, u_mip);


	addMipsShader.shader.clear();
	addMipsShader.shader.loadShaderProgramFromFile(RESOURCES_PATH "shaders/postProcess/drawQuads.vert",
		RESOURCES_PATH "shaders/postProcess/addMipsShader.frag");
	GET_UNIFORM2(addMipsShader, u_texture);
	GET_UNIFORM2(addMipsShader, u_mip);


	gausianBLurShader.shader.clear();
	gausianBLurShader.shader.loadShaderProgramFromFile(RESOURCES_PATH "shaders/postProcess/drawQuads.vert",
		RESOURCES_PATH "shaders/postProcess/gausianBlur.frag");
	GET_UNIFORM2(gausianBLurShader, u_horizontal);
	GET_UNIFORM2(gausianBLurShader, u_mip);
	GET_UNIFORM2(gausianBLurShader, u_texel);


#pragma endregion


#pragma region zpass
	{

		zpassShader.shader.clear();

		zpassShader.shader.loadShaderProgramFromFile(RESOURCES_PATH "shaders/rendering/zpass.vert",
			RESOURCES_PATH "shaders/rendering/zpass.frag");
		zpassShader.shader.bind();

		GET_UNIFORM2(zpassShader, u_viewProjection);
		GET_UNIFORM2(zpassShader, u_positionInt);
		GET_UNIFORM2(zpassShader, u_positionFloat);
		GET_UNIFORM2(zpassShader, u_renderOnlyWater);
		GET_UNIFORM2(zpassShader, u_timeGrass);

		zpassShader.u_vertexData = getStorageBlockIndex(zpassShader.shader.id, "u_vertexData");
		glShaderStorageBlockBinding(zpassShader.shader.id, zpassShader.u_vertexData, 1);

		zpassShader.u_vertexUV = getStorageBlockIndex(zpassShader.shader.id, "u_vertexUV");
		glShaderStorageBlockBinding(zpassShader.shader.id, zpassShader.u_vertexUV, 2);

		zpassShader.u_textureSamplerers = getStorageBlockIndex(zpassShader.shader.id, "u_textureSamplerers");
		glShaderStorageBlockBinding(zpassShader.shader.id, zpassShader.u_textureSamplerers, 3);
	}
#pragma endregion

#pragma region ssr
	{
		ssrShader.shader.clear();

		ssrShader.shader.loadShaderProgramFromFile(RESOURCES_PATH "shaders/postProcess/drawQuads.vert",
			RESOURCES_PATH "shaders/postProcess/ssr.frag");
		ssrShader.shader.bind();

		GET_UNIFORM2(ssrShader, u_lastFrameColor);
		GET_UNIFORM2(ssrShader, u_lastFramePositionViewSpace);
		GET_UNIFORM2(ssrShader, u_cameraProjection);
		GET_UNIFORM2(ssrShader, u_inverseView);
		GET_UNIFORM2(ssrShader, u_view);
		GET_UNIFORM2(ssrShader, u_inverseCameraViewProjection);
		GET_UNIFORM2(ssrShader, u_normals);
		
	}
#pragma endregion


#pragma region decals
	{

		decalShader.shader.clear();

		decalShader.shader.loadShaderProgramFromFile(RESOURCES_PATH "shaders/rendering/zpass.vert",
			RESOURCES_PATH "shaders/rendering/decal.frag");
		decalShader.shader.bind();

		GET_UNIFORM2(decalShader, u_viewProjection);
		GET_UNIFORM2(decalShader, u_positionInt);
		GET_UNIFORM2(decalShader, u_positionFloat);
		GET_UNIFORM2(decalShader, u_renderOnlyWater);
		GET_UNIFORM2(decalShader, u_timeGrass);
		GET_UNIFORM2(decalShader, u_zBias);
		GET_UNIFORM2(decalShader, u_crackPosition);
		

		decalShader.u_vertexData = getStorageBlockIndex(decalShader.shader.id, "u_vertexData");
		glShaderStorageBlockBinding(decalShader.shader.id, decalShader.u_vertexData, 1);

		decalShader.u_vertexUV = getStorageBlockIndex(decalShader.shader.id, "u_vertexUV");
		glShaderStorageBlockBinding(decalShader.shader.id, decalShader.u_vertexUV, 2);

		decalShader.u_textureSamplerers = getStorageBlockIndex(decalShader.shader.id, "u_textureSamplerers");
		glShaderStorageBlockBinding(decalShader.shader.id, decalShader.u_textureSamplerers, 3);


	}
#pragma endregion


#pragma region basic entity renderer

	entityRenderer.blockEntityshader.shader.clear();

	entityRenderer.blockEntityshader.shader.loadShaderProgramFromFile
	(RESOURCES_PATH "shaders/blockEntity.vert", RESOURCES_PATH "shaders/blockEntity.frag");
	entityRenderer.blockEntityshader.shader.bind();

	GET_UNIFORM2(entityRenderer.blockEntityshader, u_entityPositionInt);
	GET_UNIFORM2(entityRenderer.blockEntityshader, u_entityPositionFloat);
	GET_UNIFORM2(entityRenderer.blockEntityshader, u_viewProjection);
	GET_UNIFORM2(entityRenderer.blockEntityshader, u_modelMatrix);
	GET_UNIFORM2(entityRenderer.blockEntityshader, u_cameraPositionInt);
	GET_UNIFORM2(entityRenderer.blockEntityshader, u_cameraPositionFloat);
	GET_UNIFORM2(entityRenderer.blockEntityshader, u_texture);
	GET_UNIFORM2(entityRenderer.blockEntityshader, u_view);
	GET_UNIFORM2(entityRenderer.blockEntityshader, u_lightValue);


	//item entity shader
	//
	entityRenderer.itemEntityShader.shader.clear();

	entityRenderer.itemEntityShader.shader.loadShaderProgramFromFile
		(RESOURCES_PATH "shaders/blockEntity.vert", RESOURCES_PATH "shaders/itemEntity.frag");
	entityRenderer.itemEntityShader.shader.bind();

	GET_UNIFORM2(entityRenderer.itemEntityShader, u_entityPositionInt);
	GET_UNIFORM2(entityRenderer.itemEntityShader, u_entityPositionFloat);
	GET_UNIFORM2(entityRenderer.itemEntityShader, u_viewProjection);
	GET_UNIFORM2(entityRenderer.itemEntityShader, u_modelMatrix);
	GET_UNIFORM2(entityRenderer.itemEntityShader, u_cameraPositionInt);
	GET_UNIFORM2(entityRenderer.itemEntityShader, u_cameraPositionFloat);
	GET_UNIFORM2(entityRenderer.itemEntityShader, u_texture);
	GET_UNIFORM2(entityRenderer.itemEntityShader, u_view);
	GET_UNIFORM2(entityRenderer.itemEntityShader, u_lightValue);
	//


	entityRenderer.basicEntityShader.shader.clear();
	entityRenderer.basicEntityShader.shader.loadShaderProgramFromFile
	(RESOURCES_PATH "shaders/basicEntity.vert", RESOURCES_PATH "shaders/basicEntity.frag");
	entityRenderer.basicEntityShader.shader.bind();

	GET_UNIFORM2(entityRenderer.basicEntityShader, u_cameraPositionInt);
	GET_UNIFORM2(entityRenderer.basicEntityShader, u_cameraPositionFloat);
	GET_UNIFORM2(entityRenderer.basicEntityShader, u_viewProjection);
	GET_UNIFORM2(entityRenderer.basicEntityShader, u_modelMatrix);
	GET_UNIFORM2(entityRenderer.basicEntityShader, u_view);
	GET_UNIFORM2(entityRenderer.basicEntityShader, u_bonesPerModel);
	GET_UNIFORM2(entityRenderer.basicEntityShader, u_exposure);
	
	//
	entityRenderer.basicEntityShader.u_entityTextureSamplerers = getStorageBlockIndex(entityRenderer.basicEntityShader.shader.id, "u_entityTextureSamplerers");
	glShaderStorageBlockBinding(entityRenderer.basicEntityShader.shader.id, entityRenderer.basicEntityShader.u_entityTextureSamplerers, 5);

	entityRenderer.basicEntityShader.u_skinningMatrix = getStorageBlockIndex(entityRenderer.basicEntityShader.shader.id, "u_skinningMatrix");
	glShaderStorageBlockBinding(entityRenderer.basicEntityShader.shader.id, entityRenderer.basicEntityShader.u_skinningMatrix, 6);

	entityRenderer.basicEntityShader.u_perEntityData = getStorageBlockIndex(entityRenderer.basicEntityShader.shader.id, "u_perEntityData");
	glShaderStorageBlockBinding(entityRenderer.basicEntityShader.shader.id, entityRenderer.basicEntityShader.u_perEntityData, 7);


#pragma endregion

#pragma region post process

	hbaoShader.shader.clear();
	hbaoShader.shader.loadShaderProgramFromFile(
		RESOURCES_PATH "shaders/postProcess/drawQuads.vert",
		RESOURCES_PATH "shaders/postProcess/hbao.frag");

	GET_UNIFORM2(hbaoShader, u_gPosition);
	GET_UNIFORM2(hbaoShader, u_gNormal);
	GET_UNIFORM2(hbaoShader, u_view);
	GET_UNIFORM2(hbaoShader, u_projection);

	applyHBAOShader.shader.clear();
	applyHBAOShader.shader.loadShaderProgramFromFile(
		RESOURCES_PATH "shaders/postProcess/drawQuads.vert",
		RESOURCES_PATH "shaders/postProcess/applyHBAO.frag");

	GET_UNIFORM2(applyHBAOShader, u_hbao);
	GET_UNIFORM2(applyHBAOShader, u_currentViewSpace);

	applyHBAOShader.u_shadingSettings
		= glGetUniformBlockIndex(applyHBAOShader.shader.id, "ShadingSettings");
	glBindBufferBase(GL_UNIFORM_BUFFER,
		applyHBAOShader.u_shadingSettings, defaultShader.shadingSettingsBuffer);


	warpShader.shader.clear();
	warpShader.shader.loadShaderProgramFromFile(
		RESOURCES_PATH "shaders/postProcess/drawQuads.vert",
		RESOURCES_PATH "shaders/postProcess/warp.frag");
	GET_UNIFORM2(warpShader, u_color);
	GET_UNIFORM2(warpShader, u_time);
	GET_UNIFORM2(warpShader, u_underwaterColor);
	GET_UNIFORM2(warpShader, u_currentViewSpace);


	applyToneMapper.shader.clear();
	applyToneMapper.shader.loadShaderProgramFromFile(
		RESOURCES_PATH "shaders/postProcess/drawQuads.vert",
		RESOURCES_PATH "shaders/postProcess/toneMap.frag");
	GET_UNIFORM2(applyToneMapper, u_color);
	GET_UNIFORM2(applyToneMapper, u_tonemapper);
	GET_UNIFORM2(applyToneMapper, u_exposure);





#pragma endregion
	
}


struct DrawElementsIndirectCommand
{
	GLuint count; // The number of elements to be rendered.
	GLuint instanceCount; // The number of instances of the specified range of elements to be rendered.
	GLuint firstIndex; // The starting index in the enabled arrays.
	GLuint baseVertex; // The starting vertex index in the vertex buffer.
	GLuint baseInstance; // The base instance for use in fetching instanced vertex attributes.
};


void Renderer::renderFromBakedData(SunShadow &sunShadow, ChunkSystem &chunkSystem, Camera &c,
	ProgramData &programData, BlocksLoader &blocksLoader, ClientEntityManager &entityManager, 
	ModelsManager &modelsManager, 
	AdaptiveExposure &adaptiveExposure,
	bool showLightLevels, glm::dvec3 pointPos, bool underWater, int screenX, int screenY,
	float deltaTime, float dayTime, 
	GLuint64 currentSkinBindlessTexture, bool &playerClicked, float playerRunning,
	BoneTransform &playerHand, int currentHeldItemIndex, float waterDropsStrength,
	bool showHand
	)
{

	auto &shadingSettings = getShadingSettings();

	defaultShader.shadingSettings.exposure = adaptiveExposure.currentExposure
		+ shadingSettings.exposure;

	//glPolygonMode(GL_FRONT, GL_POINT);
	//glPolygonMode(GL_BACK, GL_FILL);
	//glEnable(GL_CULL_FACE);
	//glCullFace(GL_FRONT);
	//glFrontFace(GL_CW);

	glViewport(0, 0, screenX, screenY);


	
	if (dayTime > 1.f)
		{ dayTime -= (int)dayTime; }
	glm::vec3 mainLightPosition = sunPos;
	if (dayTime > 0.50)
	{
		mainLightPosition = -sunPos;
	}

	//shadow
	{
		sunShadow.update();

		if (programData.renderer.defaultShader.shadingSettings.shadows)
		{
			programData.renderer.renderShadow(sunShadow,
				chunkSystem, c, programData, mainLightPosition);
		}

		sunShadow.renderShadowIntoTexture(c);
	}


	glViewport(0, 0, screenX, screenY);



	fboCoppy.updateSize(screenX, screenY);
	fboCoppy.clearFBO();

	//filteredBloomColor.updateSize(screenX, screenY);
	//filteredBloomColor.clearFBO();


	if (filterBloomSize != glm::ivec2(screenX, screenY))
	{
		filterBloomSize = glm::ivec2(screenX, screenY);

		int w = screenX;
		int h = screenY;

		//16 = max mips
		int mips = 0;
		{
			int mipW = w / 2;
			int mipH = h / 2;
			for (int i = 0; i < 16; i++)
			{
				if (mipW <= 4 || mipH <= 4 ||
					(mipW < 12 && mipH < 12)
					)
				{
					break;
				}

				mipW /= 2;
				mipH /= 2;
				mips = i;
			}
		}

		{
			currentMips = mips;

			for (int i = 0; i < 2; i++)
			{
				glBindTexture(GL_TEXTURE_2D, bluredColorBuffer[i]);

				int mipW = w / 2;
				int mipH = h / 2;

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, mips);


				for (int m = 0; m <= mips; m++)
				{
					glTexImage2D(GL_TEXTURE_2D, m, GL_R11F_G11F_B10F, mipW, mipH, 0, GL_RGB, GL_FLOAT, NULL);

					mipW = mipW /= 2;
					mipH = mipH /= 2;
				}
			}
		}

	}


	fboMain.updateSize(screenX, screenY);
	fboMain.clearFBO();

	fboLastFrame.updateSize(screenX, screenY);
	fboLastFramePositions.updateSize(screenX, screenY);
	fboHBAO.updateSize(screenX / 2, screenY / 2);

	fboSkyBox.updateSize(screenX, screenY);
	fboSkyBox.clearFBO();

	glm::vec3 posFloat = {};
	glm::ivec3 posInt = {};
	glm::ivec3 blockPosition = from3DPointToBlock(c.position);
	c.decomposePosition(posFloat, posInt);

	auto viewMatrix = c.getViewMatrix();
	auto projectionMatrix = c.getProjectionMatrix();
	auto vp = projectionMatrix * viewMatrix;

	//todo change, also reuse in the the decal shader
	float timeGrass = std::clock() / 1000.f;

	auto doDayLightCalculations = [dayTime](auto dayValue, auto nightValue, auto twilightValue)
	{

		auto firstLight = dayValue;
		auto secondLight = dayValue;
		float mix = 0;

		float bias = 2;

		if (dayTime <= 0.25)
		{
			firstLight = twilightValue;
			secondLight = dayValue;
			mix = std::powf(dayTime / 0.25f, 1.f / bias);
		}
		else if (dayTime <= 0.50)
		{
			firstLight = dayValue;
			secondLight = twilightValue;
			mix = std::powf((dayTime - 0.25f) / 0.25f, bias);
		}
		else if (dayTime <= 0.75)
		{
			firstLight = twilightValue;
			secondLight = nightValue;
			mix = std::powf((dayTime - 0.50f) / 0.25f, 1.f / bias);
		}
		else if (dayTime <= 1.f)
		{
			firstLight = nightValue;
			secondLight = twilightValue;
			mix = std::powf((dayTime - 0.75f) / 0.25f, bias);
		}

		return glm::mix(firstLight, secondLight, mix);
		//return (firstLight * (1.f - mix)) + (secondLight * mix);
	};


	//determine sky light intensity.
	int skyLightIntensity = 15;
	{
		float daySkyLight = 15;
		float nightSkyLight = 3;
		float twilightLight = 9;

		skyLightIntensity = std::roundf(doDayLightCalculations(daySkyLight, nightSkyLight, twilightLight));
		
		//std::cout << skyLightIntensity << "\n";
	}

	glm::vec3 sunLightColor = {1,1,1};
	{

		glm::vec3 daySkyLight(1.5f);
		glm::vec3 nightSkyLight = (glm::vec3(47, 135, 244) / 255.f) * 0.50f;
		glm::vec3 twilightLight = (glm::vec3(255, 159, 107) / 255.f) * 0.2f;
		
		sunLightColor = doDayLightCalculations(daySkyLight, nightSkyLight, twilightLight);
	}

	glm::vec3 ambientColor = {1,1,1};
	{
		glm::vec3 daySkyLight(1.0f);
		glm::vec3 nightSkyLight = (glm::vec3(47, 135, 244) / 255.f) * 0.9f;
		glm::vec3 twilightLight = (glm::vec3(255, 159, 107) / 255.f) * 0.6f;

		ambientColor = doDayLightCalculations(daySkyLight, nightSkyLight, twilightLight);
	}

	

#pragma region frustum culling and sorting

	//chunk vector copy has only valid non culled chunks!
	std::vector<Chunk*> chunkVectorCopy;
	chunkVectorCopy.reserve(chunkSystem.loadedChunks.size());

	FrustumVolume cameraFrustum(c.getViewProjectionWithPositionMatrixDouble());

	for (auto c : chunkSystem.loadedChunks)
	{
		if (c)
		{
			bool culled = 0;
			c->setCulled(0);

			if (c->isDontDrawYet())
			{
				culled = 1;
			}else
			if (frustumCulling)
			{
				AABBVolume chunkAABB;

				chunkAABB.minVertex = {c->data.x * CHUNK_SIZE, 0, c->data.z * CHUNK_SIZE};
				chunkAABB.maxVertex = {(c->data.x + 1) * CHUNK_SIZE, CHUNK_HEIGHT + 1, (c->data.z + 1) * CHUNK_SIZE};

				if (!CheckFrustumVsAABB(cameraFrustum, chunkAABB))
				{
					c->setCulled(1);
					culled = 1;
				}
			};

			if (!culled)
			{
				chunkVectorCopy.push_back(c);
			}
		}
	}

	//sort chunks
	{
		std::sort(chunkVectorCopy.begin(), chunkVectorCopy.end(),
			[x = divideChunk(blockPosition.x), z = divideChunk(blockPosition.z)](Chunk *b, Chunk *a)
		{
			if (a == nullptr) { return false; }
			if (b == nullptr) { return true; }

			int ax = a->data.x - x;
			int az = a->data.z - z;

			int bx = b->data.x - x;
			int bz = b->data.z - z;

			unsigned long reza = ax * ax + az * az;
			unsigned long rezb = bx * bx + bz * bz;

			return reza < rezb;
		}
		);
	}

#pragma endregion



#pragma region render sky box 0

	glBindFramebuffer(GL_FRAMEBUFFER, fboMain.fbo);

	programData.skyBoxLoaderAndDrawer.drawBefore(c.getProjectionMatrix() * c.getViewMatrix(),
		mainLightPosition, dayTime);

	fboSkyBox.copyColorFromOtherFBO(fboMain.color,
		fboMain.size.x, fboMain.size.y);


#pragma endregion


#pragma region setup uniforms and stuff
	{

		defaultShader.shadingSettings.fogDistance =
			(chunkSystem.squareSize / 2.f) * CHUNK_SIZE - CHUNK_SIZE;


		glNamedBufferData(defaultShader.shadingSettingsBuffer,
			sizeof(defaultShader.shadingSettings), &defaultShader.shadingSettings,
			GL_STREAM_DRAW);

		zpassShader.shader.bind();
		glUniformMatrix4fv(zpassShader.u_viewProjection, 1, GL_FALSE, &vp[0][0]);
		glUniform3fv(zpassShader.u_positionFloat, 1, &posFloat[0]);
		glUniform3iv(zpassShader.u_positionInt, 1, &posInt[0]);
		glUniform1i(zpassShader.u_renderOnlyWater, 0);
		glUniform1f(zpassShader.u_timeGrass, timeGrass);



		defaultShader.shader.bind();
		glUniformMatrix4fv(defaultShader.u_viewProjection, 1, GL_FALSE, &vp[0][0]);
		glUniform3fv(defaultShader.u_positionFloat, 1, &posFloat[0]);
		glUniform3iv(defaultShader.u_positionInt, 1, &posInt[0]);
		glUniform1i(defaultShader.u_typesCount, BlocksCount);	//remove
		glUniform1f(defaultShader.u_time, std::clock() / 400.f);
		glUniform1i(defaultShader.u_showLightLevels, showLightLevels);
		glUniform1i(defaultShader.u_skyLightIntensity, skyLightIntensity);
		glUniform3fv(defaultShader.u_sunDirection, 1, &mainLightPosition[0]);
		glUniform1f(defaultShader.u_metallic, metallic);
		glUniform1f(defaultShader.u_roughness, roughness);
		glUniform1i(defaultShader.u_underWater, underWater);
		glUniform3fv(defaultShader.u_sunLightColor, 1, &sunLightColor[0]);
		glUniform3fv(defaultShader.u_ambientColor, 1, &ambientColor[0]);
		glUniform3fv(defaultShader.u_waterColor, 1, &defaultShader.shadingSettings.waterColor[0]);
		glUniform1i(defaultShader.u_depthPeelwaterPass, 0);
		glUniform1i(defaultShader.u_hasPeelInformation, 0);
		glUniform1f(defaultShader.u_waterMove, waterTimer);
		glUniform1f(defaultShader.u_near, c.closePlane);
		glUniform1f(defaultShader.u_far, c.farPlane);
		glUniformMatrix4fv(defaultShader.u_inverseProjMat, 1, 0,
			&glm::inverse(projectionMatrix)[0][0]);
		glUniformMatrix4fv(defaultShader.u_lightSpaceMatrix, 1, 0,
			&sunShadow.lightSpaceMatrix[0][0]);
		glUniform3iv(defaultShader.u_lightPos, 1, &sunShadow.lightSpacePosition[0]);
		glUniform1i(defaultShader.u_writeScreenSpacePositions, 1);//todo remove

		glUniformMatrix4fv(defaultShader.u_inverseViewProjMat, 1, 0,
			&glm::inverse(projectionMatrix * viewMatrix)[0][0]);

		glUniformMatrix4fv(defaultShader.u_lastViewProj, 1, 0,
			&(c.lastFrameViewProjMatrix)[0][0]);

	#pragma region textures
		programData.numbersTexture.bind(0);

		programData.dudv.bind(1);
		glUniform1i(defaultShader.u_dudv, 1);

		programData.dudvNormal.bind(2);
		glUniform1i(defaultShader.u_dudvNormal, 2);

		programData.causticsTexture.bind(3);
		glUniform1i(defaultShader.u_caustics, 3);

		glActiveTexture(GL_TEXTURE0 + 4);
		glBindTexture(GL_TEXTURE_2D, sunShadow.shadowMap.depth);
		glUniform1i(defaultShader.u_sunShadowTexture, 4);

		glActiveTexture(GL_TEXTURE0 + 5);
		glBindTexture(GL_TEXTURE_2D, fboLastFrame.color);
		glUniform1i(defaultShader.u_lastFrameColor, 5);

		glActiveTexture(GL_TEXTURE0 + 6);
		glBindTexture(GL_TEXTURE_2D, fboLastFramePositions.color);
		glUniform1i(defaultShader.u_lastFramePositionViewSpace, 6);

		glActiveTexture(GL_TEXTURE0 + 7);
		glBindTexture(GL_TEXTURE_2D, programData.brdfTexture.id);
		glUniform1i(defaultShader.u_brdf, 7);

		glActiveTexture(GL_TEXTURE0 + 8);
		glBindTexture(GL_TEXTURE_2D, fboSkyBox.color);
		glUniform1i(defaultShader.u_skyTexture, 8);

		glActiveTexture(GL_TEXTURE0 + 11);
		glBindTexture(GL_TEXTURE_2D, programData.aoTexture.id);
		glUniform1i(defaultShader.u_ao, 11);
	#pragma endregion


		glUniform1f(defaultShader.u_baseAmbientExtra, adaptiveExposure.bonusAmbient);

		glUniform1f(defaultShader.u_timeGrass, timeGrass);

		glUniformMatrix4fv(defaultShader.u_cameraProjection, 1, GL_FALSE, glm::value_ptr(c.getProjectionMatrix()));

		glUniformMatrix4fv(defaultShader.u_inverseView, 1, GL_FALSE,
			glm::value_ptr(glm::inverse(viewMatrix)));

		glUniformMatrix4fv(defaultShader.u_view, 1, GL_FALSE,
			glm::value_ptr(viewMatrix));

		waterTimer += deltaTime * 0.09;
		if (waterTimer > 20)
		{
			waterTimer -= 20;
		}

		//waterTimer += deltaTime * 0.4;
		//if (waterTimer > 1)
		//{
		//	waterTimer -= 1;
		//}


		{
			glm::ivec3 i;
			glm::vec3 f;
			decomposePosition(pointPos, f, i);

			glUniform3fv(defaultShader.u_pointPosF, 1, &f[0]);
			glUniform3iv(defaultShader.u_pointPosI, 1, &i[0]);
		}

	#pragma region lights
		{

			if (chunkSystem.shouldUpdateLights)
			{
				chunkSystem.shouldUpdateLights = 0;

				//std::cout << "Updated lights\n";

				lightsBufferCount = 0;

				for (auto c : chunkSystem.loadedChunks)
				{
					if (c && !c->isDontDrawYet())
					{
						lightsBufferCount += c->lightsElementCountSize;
					}
				}

				glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightBuffer);
				glBufferData(GL_SHADER_STORAGE_BUFFER, lightsBufferCount * sizeof(glm::ivec4), NULL, GL_STREAM_COPY);

				glBindBuffer(GL_COPY_WRITE_BUFFER, lightBuffer);

				//todo only copy chunks that are close
				//todo max lights
				size_t offset = 0;
				for (auto c : chunkSystem.loadedChunks)
				{

					if (c && !c->isDontDrawYet())
					{
						glBindBuffer(GL_COPY_READ_BUFFER, c->lightsBuffer);

						// Copy data from the first existing SSBO to the new SSBO
						glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, offset,
							c->lightsElementCountSize * sizeof(glm::ivec4));

						offset += c->lightsElementCountSize * sizeof(glm::ivec4);
					}
				}

				glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightBuffer);
				glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, lightBuffer);
			};

			glUniform1i(defaultShader.u_lightsCount, lightsBufferCount);

			//auto c = chunkSystem.getChunkSafeFromBlockPos(posInt.x, posInt.z);
			//
			//if (c)
			//{
			//	glBindBuffer(GL_SHADER_STORAGE_BUFFER, c->lightsBuffer);
			//	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, c->lightsBuffer);
			//	glUniform1i(u_lightsCount, c->lightsElementCountSize);
			//}
		}
	#pragma endregion

	}
#pragma endregion


	//configure opaque geometry
	static std::vector<DrawElementsIndirectCommand> drawCommands;

	if(unifiedGeometry)
	{
		glBindBuffer(GL_DRAW_INDIRECT_BUFFER, drawCommandsOpaqueBuffer);
		drawCommands.clear();

		if (sortChunks)
		{
			int s = chunkVectorCopy.size();
			for (int i = s - 1; i >= 0; i--)
			{
				auto chunk = chunkVectorCopy[i];
				
				int facesCount = chunk->elementCountSize;
				if (facesCount)
				{

					auto entry = chunkSystem.gpuBuffer
						.getEntry({chunk->data.x,chunk->data.z});

					permaAssertComment(
						(int)entry.size / (4 * sizeof(int)) == facesCount,
						"Gib Gpu Buffer desync");

					// Prepare draw command for this chunk
					DrawElementsIndirectCommand command;
					command.count = 4; // Assuming you're drawing quads
					command.instanceCount = facesCount;
					command.firstIndex = 0;
					command.baseVertex = 0;
					command.baseInstance = entry.beg / (4 * sizeof(int)); 

					// Add draw command to the array
					drawCommands.push_back(command);

				}
			}
		}
		else
		{
			for (auto &chunk : chunkVectorCopy)
			{
				int facesCount = chunk->elementCountSize;
				if (facesCount)
				{
					auto entry = chunkSystem.gpuBuffer
						.getEntry({chunk->data.x,chunk->data.z});

					permaAssertComment(
						(int)entry.size / (4 * sizeof(int)) == facesCount,
						"Gib Gpu Buffer desync");

					// Prepare draw command for this chunk
					DrawElementsIndirectCommand command;
					command.count = 4; // Assuming you're drawing quads
					command.instanceCount = facesCount;
					command.firstIndex = 0;
					command.baseVertex = 0;
					command.baseInstance = entry.beg / (4 * sizeof(int));

					// Add draw command to the array
					drawCommands.push_back(command);

				}
			}
		}

		glBindBuffer(GL_DRAW_INDIRECT_BUFFER, drawCommandsOpaqueBuffer);
		glBufferData(GL_DRAW_INDIRECT_BUFFER, drawCommands.size() * sizeof(DrawElementsIndirectCommand), drawCommands.data(), GL_STREAM_DRAW);
	}

	auto renderStaticGeometry = [&]()
	{

		if (unifiedGeometry)
		{
			glBindBuffer(GL_DRAW_INDIRECT_BUFFER, drawCommandsOpaqueBuffer);

			glBindVertexArray(chunkSystem.gpuBuffer.vao);

			glMultiDrawElementsIndirect(GL_TRIANGLE_FAN, GL_UNSIGNED_BYTE,
				nullptr, drawCommands.size(), sizeof(DrawElementsIndirectCommand));
			
		}
		else
		{

			//this is just for a spped cmparison test
			if (sortChunks)
			{
				int s = chunkVectorCopy.size();
				for (int i = s - 1; i >= 0; i--)
				{
					auto chunk = chunkVectorCopy[i];
					int facesCount = chunk->elementCountSize;
					if (facesCount)
					{
						glBindVertexArray(chunk->vao);
						glDrawElementsInstanced(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_BYTE, 0, facesCount);
					}
				}
			}
			else
			{
				for (auto &chunk : chunkVectorCopy)
				{
					int facesCount = chunk->elementCountSize;
					if (facesCount)
					{
						glBindVertexArray(chunk->vao);
						glDrawElementsInstanced(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_BYTE, 0, facesCount);
					}
				}
			}

		}

	};

	auto renderTransparentGeometry = [&]()
	{
		if (!renderTransparent) { return; }

		for (auto &chunk : chunkVectorCopy)
		{
			int facesCount = chunk->transparentElementCountSize;
			if (facesCount)
			{
				glBindVertexArray(chunk->transparentVao);
				glDrawElementsInstanced(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_BYTE, 0, facesCount);
			}
		}
	};

	auto depthPrePass = [&]()
	{
		if (zprepass)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, fboMain.fbo);
			glDepthFunc(GL_LESS);
			glDisable(GL_BLEND);
			glColorMask(0, 0, 0, 0);
			zpassShader.shader.bind();
			renderStaticGeometry();
		}
	};
	
	auto solidPass = [&]()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, fboMain.fbo);
		glColorMask(1, 1, 1, 1);

		if (zprepass)
		{
			glDepthFunc(GL_EQUAL);
		}
		else
		{
			glDepthFunc(GL_LESS);
		}

		defaultShader.shader.bind();
		glDisable(GL_BLEND);
		//glEnable(GL_BLEND);
		//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		renderStaticGeometry();
	};

	auto copyToMainFbo = [&]()
	{
		fboMain.writeAllToOtherFbo(0, screenX, screenY);
		fboLastFrame.copyColorFromOtherFBO(fboMain.fboOnlyFirstTarget, screenX, screenY);
		fboLastFramePositions.copyColorFromOtherFBO(fboMain.fboOnlySecondTarget, screenX, screenY);
	};

	auto copyToMainFboOnlyLastFrameStuff = [&]()
	{
		fboLastFrame.copyColorFromOtherFBO(fboMain.fboOnlyFirstTarget, screenX, screenY);
		fboLastFramePositions.copyColorFromOtherFBO(fboMain.fboOnlySecondTarget, screenX, screenY);
	};

	auto renderTransparentGeometryPhaze = [&](bool hasPeelInformation)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, fboMain.fbo);
		defaultShader.shader.bind();
		glColorMask(1, 1, 1, 1);

		glEnablei(GL_BLEND, 0);
		glBlendFunci(0, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDisablei(GL_BLEND, 1);


		glColorMask(1, 1, 1, 1);
		glUniform1i(defaultShader.u_depthPeelwaterPass, 0);
		glUniform1i(defaultShader.u_hasPeelInformation, hasPeelInformation);

		glActiveTexture(GL_TEXTURE0 + 10);
		glBindTexture(GL_TEXTURE_2D, fboCoppy.color);
		glUniform1i(defaultShader.u_PeelTexture, 10);

		glActiveTexture(GL_TEXTURE0 + 9);
		glBindTexture(GL_TEXTURE_2D, fboCoppy.depth);
		glUniform1i(defaultShader.u_depthTexture, 9);


		glDepthFunc(GL_LESS);
		glDisable(GL_CULL_FACE); //todo change
		renderTransparentGeometry();
		glEnable(GL_CULL_FACE);
	};

	int queryDataForSunFlare = 0;

	if (getShadingSettings().waterType)
	{

	#pragma region depth pre pass 1
		programData.GPUProfiler.startSubProfile("depth pre pass 1");
		depthPrePass();
		programData.GPUProfiler.endSubProfile("depth pre pass 1");
	#pragma endregion



	#pragma region solid pass 2
		programData.GPUProfiler.startSubProfile("solid pass 2");
		solidPass();
		programData.GPUProfiler.endSubProfile("solid pass 2");
	#pragma endregion


	#pragma region render entities
		programData.GPUProfiler.startSubProfile("entities");
		renderEntities(deltaTime, c, modelsManager, blocksLoader,
			entityManager, vp, c.getProjectionMatrix(), viewMatrix, posFloat, posInt,
			programData.renderer.defaultShader.shadingSettings.exposure, chunkSystem, skyLightIntensity,
			currentSkinBindlessTexture, playerClicked, playerRunning, playerHand, currentHeldItemIndex,
			showHand);
		programData.GPUProfiler.endSubProfile("entities");
	#pragma endregion


	//copy depth 3
		fboCoppy.copyDepthFromOtherFBO(fboMain.fbo, screenX, screenY);
	


		//render sun moon
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glEnablei(GL_BLEND, 0);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		glBindFramebuffer(GL_FRAMEBUFFER, fboMain.fbo);

		queryDataForSunFlare = sunFlareQueries[sunFlareQueryPos].getQueryResult();

		sunFlareQueries[sunFlareQueryPos].begin();
		programData.sunRenderer.render(c, sunPos, programData.skyBoxLoaderAndDrawer.sunTexture);
		sunFlareQueries[sunFlareQueryPos].end();
		sunFlareQueryPos++;
		sunFlareQueryPos %= (sizeof(sunFlareQueries) / sizeof(sunFlareQueries[0]));


		programData.sunRenderer.render(c, -sunPos, programData.skyBoxLoaderAndDrawer.moonTexture);
		glDisable(GL_BLEND);
		glDepthFunc(GL_LESS);


		//disable bloom for transparent geometry
		{
			glBindFramebuffer(GL_FRAMEBUFFER, fboMain.fbo);
			unsigned int attachments[3] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1,
			GL_COLOR_ATTACHMENT2};
			glDrawBuffers(3, attachments);
		}

	#pragma region render only water geometry to depth 4
		programData.GPUProfiler.startSubProfile("render only water to depth 4");
		defaultShader.shader.bind();
		glBindFramebuffer(GL_FRAMEBUFFER, fboCoppy.fbo);
		glDepthFunc(GL_LESS);
		zpassShader.shader.bind();
		glUniform1i(zpassShader.u_renderOnlyWater, 1);
		glEnablei(GL_BLEND, 0);
		glBlendFunci(0, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDisablei(GL_BLEND, 1);
		glColorMask(0, 0, 0, 0);

		renderTransparentGeometry();
		programData.GPUProfiler.endSubProfile("render only water to depth 4");
	#pragma endregion


	#pragma region render with depth peel first part of the transparent of the geometry 5
		programData.GPUProfiler.startSubProfile("render first water 5");
		glBindFramebuffer(GL_FRAMEBUFFER, fboMain.fbo);
		defaultShader.shader.bind();

		glEnablei(GL_BLEND, 0);
		glBlendFunci(0, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDisablei(GL_BLEND, 1);

		glColorMask(1, 1, 1, 1);
		glUniform1i(defaultShader.u_depthPeelwaterPass, true);

		glActiveTexture(GL_TEXTURE0 + 9);
		glBindTexture(GL_TEXTURE_2D, fboCoppy.depth);
		glUniform1i(defaultShader.u_depthTexture, 9);

		glDepthFunc(GL_LESS);
		//glDisable(GL_CULL_FACE); //todo change
		//todo disable ssr for this step?
		renderTransparentGeometry();
		programData.GPUProfiler.endSubProfile("render first water 5");
	#pragma endregion


	#pragma region copy color buffer and last depth for later use 6
		fboCoppy.copyDepthAndColorFromOtherFBO(fboMain.fbo, screenX, screenY);
	#pragma endregion

		
	#pragma region render transparent geometry last phaze 7
		programData.GPUProfiler.startSubProfile("final transparency 7");

		renderTransparentGeometryPhaze(true);
		programData.GPUProfiler.endSubProfile("final transparency 7");
	#pragma endregion

		{
			glBindFramebuffer(GL_FRAMEBUFFER, fboMain.fbo);
			unsigned int attachments[4] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1,
			GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3};
			glDrawBuffers(4, attachments);
		}

	}
	else
	{
		programData.GPUProfiler.startSubProfile("depth pre pass 1");
		depthPrePass();
		programData.GPUProfiler.endSubProfile("depth pre pass 1");

		
		programData.GPUProfiler.startSubProfile("solid pass 2");
		solidPass();
		programData.GPUProfiler.endSubProfile("solid pass 2");

	#pragma region render entities
		programData.GPUProfiler.startSubProfile("entities");
		renderEntities(deltaTime, c, modelsManager, blocksLoader,
			entityManager, vp, c.getProjectionMatrix(), viewMatrix, posFloat, posInt,
			programData.renderer.defaultShader.shadingSettings.exposure, chunkSystem, skyLightIntensity,
			currentSkinBindlessTexture, playerClicked, playerRunning, playerHand, currentHeldItemIndex,
			showHand);
		programData.GPUProfiler.endSubProfile("entities");
	#pragma endregion

	//copy depth 3
		fboCoppy.copyDepthFromOtherFBO(fboMain.fbo, screenX, screenY);

		//disable bloom for transparent geometry
		{
			glBindFramebuffer(GL_FRAMEBUFFER, fboMain.fbo);
			unsigned int attachments[3] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1,
			GL_COLOR_ATTACHMENT2};
			glDrawBuffers(3, attachments);
		}

		programData.GPUProfiler.startSubProfile("final transparency 7");
		renderTransparentGeometryPhaze(false);
		programData.GPUProfiler.endSubProfile("final transparency 7");

		{
			glBindFramebuffer(GL_FRAMEBUFFER, fboMain.fbo);
			unsigned int attachments[4] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1,
			GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3};
			glDrawBuffers(4, attachments);
		}

	}

#pragma region ssr

	if(0)
	{
		programData.GPUProfiler.startSubProfile("SSR PASS");

		//fboMain.color

		copyToMainFboOnlyLastFrameStuff();
		
		glBindFramebuffer(GL_FRAMEBUFFER, fboMain.fboOnlyFirstTarget);

		glDisable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		ssrShader.shader.bind();
		glBindVertexArray(vaoQuad);

		glUniformMatrix4fv(ssrShader.u_cameraProjection, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

		glUniformMatrix4fv(ssrShader.u_inverseView, 1, GL_FALSE,
			glm::value_ptr(glm::inverse(viewMatrix)));

		glUniformMatrix4fv(ssrShader.u_view, 1, GL_FALSE,
			glm::value_ptr(viewMatrix));

		glUniformMatrix4fv(ssrShader.u_inverseCameraViewProjection, 1, GL_FALSE,
			glm::value_ptr(glm::inverse(projectionMatrix *viewMatrix)));


		glActiveTexture(GL_TEXTURE0 + 5);
		glBindTexture(GL_TEXTURE_2D, fboLastFrame.color);
		glUniform1i(ssrShader.u_lastFrameColor, 5);

		glActiveTexture(GL_TEXTURE0 + 6);
		glBindTexture(GL_TEXTURE_2D, fboLastFramePositions.color);
		glUniform1i(ssrShader.u_lastFramePositionViewSpace, 6);

		glActiveTexture(GL_TEXTURE0 + 1);
		glBindTexture(GL_TEXTURE_2D, fboMain.thirdColor);
		glUniform1i(ssrShader.u_normals, 1);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		programData.GPUProfiler.endSubProfile("SSR PASS");
	}

#pragma endregion

#pragma region get automatic exposure
	if(1)
	{

		static bool reading = 0;

		static ImVec4 color = {0,0,0,1};

		//if (ImGui::Begin("client controll"))
		//{
		//	ImGui::ColorButton("Colror", color);
		//	ImGui::Text("Average Luminosity: %f", averageLuminosity);
		//	ImGui::Text("Bonus ambient: %f", adaptiveExposure.bonusAmbient);
		//}
		//ImGui::End();

		if (!reading)
		{


			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, fboMain.color);
			glGenerateMipmap(GL_TEXTURE_2D);

			GLint mipLevels = 1 + (GLint)floor(log2(std::max(screenX, screenY)));
			// Bind the texture at the last mip level
			//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, mipLevels - 1);
			//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, mipLevels - 1);

			glBindBuffer(GL_PIXEL_PACK_BUFFER, automatixExposureReadBUffer);

			glBindFramebuffer(GL_FRAMEBUFFER, fboMain.fboOnlyFirstTarget);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboMain.color, mipLevels - 1);

			glReadBuffer(GL_COLOR_ATTACHMENT0); // Assuming you are reading from a framebuffer
			glReadPixels(0, 0, 1, 1, GL_RGB, GL_FLOAT, 0); // Reading to the PBO



			//reset
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboMain.color, 0);
			//glBindTexture(GL_TEXTURE_2D, fboMain.color);
			//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
			//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, mipLevels - 1);
			reading = 1;
		}else
		{
			glBindBuffer(GL_PIXEL_PACK_BUFFER, automatixExposureReadBUffer);
			GLfloat *ptr = (GLfloat *)glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);

			if (ptr)
			{

				glm::vec3 screenColor = glm::vec3(ptr[0], ptr[1], ptr[2]);
				screenColor = glm::pow(screenColor, glm::vec3(1.f / 2.2f));

				averageLuminosity = glm::dot(screenColor, glm::vec3(0.2126, 0.7152, 0.0722));
				//std::cout << averageLuminosity << "\n";
				//std::cout << ptr[0] << " " <<  ptr[1] << " " << ptr[2] << "\n";

				color.x = screenColor[0];
				color.y = screenColor[1];
				color.z = screenColor[2];

				glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
				reading = 0;
			}
			else
			{
				//std::cout << "no\n";
			}
		}

		glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);


		//update automatic exposure
		adaptiveExposure.update(deltaTime, averageLuminosity);

	}
#pragma endregion

	bool lastBloomChannel = 0;
	if (bloom)
	{


		programData.GPUProfiler.startSubProfile("Bloom");

#pragma region get bloom filtered data
	if(bloom)
	{
		glDisable(GL_DEPTH_TEST);

		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);
		glBlendEquation(GL_FUNC_ADD);

		glBindVertexArray(vaoQuad);
		glBindFramebuffer(GL_FRAMEBUFFER, fboMain.fboOnlyFourthTarget);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, fboMain.color);
		filterBloomDataShader.shader.bind();


		glUniform1f(filterBloomDataShader.u_exposure, defaultShader.shadingSettings.exposure);

		if (underWater)
		{
			glUniform1f(filterBloomDataShader.u_tresshold, bloomTresshold / 2);
		}
		else
		{
			glUniform1f(filterBloomDataShader.u_tresshold, bloomTresshold);
		}

		glUniform1f(filterBloomDataShader.u_multiplier, bloomMultiplier);

		glViewport(0, 0, screenX, screenY);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		glDisable(GL_BLEND);

	}
#pragma endregion

#pragma region bloom blur
	if (bloom)
	{
		glBindVertexArray(vaoQuad);


		int finalMip = currentMips;

		{
			bool horizontal = 0; bool firstTime = 1;
			int mipW = screenX;
			int mipH = screenY;
			lastBloomChannel = !horizontal;

			for (int i = 0; i < currentMips + 1; i++)
			{
			#pragma region scale down
				mipW /= 2;
				mipH /= 2;
				glViewport(0, 0, mipW, mipH);

				glBindFramebuffer(GL_FRAMEBUFFER, blurFbo[horizontal]);
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
					bluredColorBuffer[horizontal], i);

				filterDownShader.shader.bind();
				glActiveTexture(GL_TEXTURE0);
				glUniform1i(filterDownShader.u_texture, 0);
				glUniform1i(filterDownShader.u_mip, firstTime ? 0 : i - 1);
				
				glBindTexture(GL_TEXTURE_2D,
					firstTime ? fboMain.fourthColor : bluredColorBuffer[lastBloomChannel]);

				glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
				lastBloomChannel = horizontal;
			#pragma endregion

			#pragma region copy data

				glBindFramebuffer(GL_FRAMEBUFFER, blurFbo[!horizontal]);
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
					bluredColorBuffer[!horizontal], i);

				addMipsShader.shader.bind();
				glActiveTexture(GL_TEXTURE0);
				glUniform1i(addMipsShader.u_texture, 0);
				glUniform1i(addMipsShader.u_mip, i);
				glBindTexture(GL_TEXTURE_2D,
					bluredColorBuffer[lastBloomChannel]);

				glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
				lastBloomChannel = !horizontal;

			#pragma endregion


			#pragma region blur
				gausianBLurShader.shader.bind();
				glActiveTexture(GL_TEXTURE0);
				//glUniform1i(gausianBLurShader.u_toBlurcolorInput, 0);
				glUniform2f(gausianBLurShader.u_texel, 1.f / mipW, 1.f / mipH);
				glUniform1i(gausianBLurShader.u_mip, i);
				//horizontal = !horizontal;

				for (int j = 0; j < 2; j++)
				{
					glBindFramebuffer(GL_FRAMEBUFFER, blurFbo[horizontal]);
					glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
						bluredColorBuffer[horizontal], i);
					glClear(GL_COLOR_BUFFER_BIT);
					glUniform1i(gausianBLurShader.u_horizontal, horizontal);

					glBindTexture(GL_TEXTURE_2D, bluredColorBuffer[lastBloomChannel]);

					glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

					lastBloomChannel = horizontal;
					horizontal = !horizontal;
					firstTime = false;
				}
				horizontal = !horizontal;

			#pragma endregion

			}
		}


		{
			glEnable(GL_BLEND);
			glBlendFunc(GL_ONE, GL_ONE);
			addMipsShader.shader.bind();

			glActiveTexture(GL_TEXTURE0);
			glUniform1i(addMipsShader.u_texture, 0);
			for (; finalMip > 0; finalMip--)
			{
				int mipW = screenX;
				int mipH = screenY;

				for (int i = 0; i < finalMip; i++)
				{
					mipW /= 2;
					mipH /= 2;
				}
				glViewport(0, 0, mipW, mipH);

				glBindFramebuffer(GL_FRAMEBUFFER, blurFbo[!lastBloomChannel]);
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
					bluredColorBuffer[!lastBloomChannel], finalMip - 1);

				glUniform1i(addMipsShader.u_mip, finalMip);
				glBindTexture(GL_TEXTURE_2D, bluredColorBuffer[lastBloomChannel]);

				glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
				lastBloomChannel = !lastBloomChannel;

			}

			glDisable(GL_BLEND);
			glViewport(0, 0, screenX, screenY);

		}
		


	}
#pragma endregion

		programData.GPUProfiler.endSubProfile("Bloom");

	};


#pragma region post process

	//hbao
	if (programData.renderer.ssao)
	{

		programData.GPUProfiler.startSubProfile("HBAO");

		glViewport(0, 0, fboHBAO.size.x, fboHBAO.size.y);

		glBindFramebuffer(GL_FRAMEBUFFER, fboHBAO.fbo);
		glClear(GL_COLOR_BUFFER_BIT);

		hbaoShader.shader.bind();
		glBindVertexArray(vaoQuad);
		
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, fboMain.secondaryColor);
		glUniform1i(hbaoShader.u_gPosition, 0);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, fboMain.thirdColor);
		glUniform1i(hbaoShader.u_gNormal, 1);

		glUniformMatrix4fv(hbaoShader.u_view, 1, GL_FALSE,
			glm::value_ptr(viewMatrix));

		glUniformMatrix4fv(hbaoShader.u_projection, 1, GL_FALSE,
			glm::value_ptr(c.getProjectionMatrix()));
		//GET_UNIFORM2(hbaoShader, u_texNoise);
		
		
		
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		//todo optimize texture binding so it is done only once at the beginning, not changed that often
		
		//apply hbao
		glDisable(GL_DEPTH_TEST);
		glViewport(0, 0, screenX, screenY);

		glBindFramebuffer(GL_FRAMEBUFFER, fboMain.fboOnlyFirstTarget);
		
		
		applyHBAOShader.shader.bind();
		glEnable(GL_BLEND);
		glBlendFunci(0, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, fboHBAO.color);
		glUniform1i(applyHBAOShader.u_hbao, 0);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, fboMain.secondaryColor);
		glUniform1i(applyHBAOShader.u_currentViewSpace, 1);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);


		programData.GPUProfiler.endSubProfile("HBAO");

	};

	GLuint currentTexture = 0;

	//bloom
	if(bloom)
	{


		glDisable(GL_DEPTH_TEST);

		applyBloomDataShader.shader.bind();

		glBindVertexArray(vaoQuad);
		glBindFramebuffer(GL_FRAMEBUFFER, fboMain.color);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, bluredColorBuffer[lastBloomChannel]);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, programData.lensDirtTexture.id);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, programData.hitDirtTexture.id);


		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, programData.waterDirtTexture.id);

		glUniform1f(applyBloomDataShader.u_waterDropsPower, waterDropsStrength);

		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);
		glBlendEquation(GL_FUNC_ADD);

		glViewport(0, 0, screenX, screenY);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		

		glDisable(GL_BLEND);

	}

	//warp
	if (underWater)
	{



		programData.GPUProfiler.startSubProfile("under water post process");

		glBindVertexArray(vaoQuad);

		glDisable(GL_BLEND);
		glDisable(GL_DEPTH_TEST);

		//glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, fboCoppy.fbo);
		currentTexture = fboCoppy.color;


		warpShader.shader.bind();

		glm::vec3 underWater = defaultShader.shadingSettings.underWaterColor;

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, fboMain.color);
		glUniform1i(warpShader.u_color, 0);
		glUniform1f(warpShader.u_time, timeGrass); //todo change
		glUniform3f(warpShader.u_underwaterColor, underWater.x,
			underWater.y, underWater.z);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, fboMain.secondaryColor);
		glUniform1i(warpShader.u_currentViewSpace, 1);


		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		//copyToMainFboOnlyLastFrameStuff();

		programData.GPUProfiler.endSubProfile("under water post process");

	}
	else
	{
		currentTexture = fboMain.color;
		//copyToMainFbo();
	}


	//tone mapping
	{

		programData.GPUProfiler.startSubProfile("tone mapping");

		glBindVertexArray(vaoQuad);

		glDisable(GL_BLEND);
		glDisable(GL_DEPTH_TEST);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);


		applyToneMapper.shader.bind();

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, currentTexture);
		glUniform1i(applyToneMapper.u_color, 0);
		glUniform1i(applyToneMapper.u_tonemapper, defaultShader.shadingSettings.tonemapper);
		glUniform1f(applyToneMapper.u_exposure, adaptiveExposure.currentExposure);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		copyToMainFboOnlyLastFrameStuff();

		programData.GPUProfiler.endSubProfile("tone mapping");

	}

	fboMain.copyDepthToMainFbo(fboMain.size.x, fboMain.size.y);

#pragma endregion


#pragma region lens flare



	{
		auto &renderer2d = programData.ui.renderer2d;
		renderer2d.pushCamera();
		

		//sunPos

		glm::vec2 screenCoords = {};
		glm::vec4 sun3Dpos = glm::vec4(sunPos, 1);
		glm::vec4 sunProjectedCoords = c.getProjectionMatrix() * c.getViewMatrix() * sun3Dpos;

		if (sunProjectedCoords.w > 0)
		{
			sunProjectedCoords.x /= sunProjectedCoords.w;
			sunProjectedCoords.y /= sunProjectedCoords.w;
			sunProjectedCoords.x += 1;
			sunProjectedCoords.x /= 2;
			sunProjectedCoords.y += 1;
			sunProjectedCoords.y /= 2;
			sunProjectedCoords.y = 1.f - sunProjectedCoords.y;


			float sunSize = 0;
			float maxSize = 0;
			{
				auto c2 = c;
				c2.viewDirection = glm::vec3(0, 0, -1);

				glm::vec4 sunEdge3Dpos1 = glm::vec4(glm::vec3(0, 0, -4), 1);
				glm::vec4 sunEdge3Dpos2 = glm::vec4(glm::vec3(1, 0, -4), 1);
				glm::vec4 sunEdgeProjectedCoords = c.getProjectionMatrix() * c2.getViewMatrix() * sunEdge3Dpos1;
				glm::vec4 sunEdgeProjectedCoords2 = c.getProjectionMatrix() * c2.getViewMatrix() * sunEdge3Dpos2;
				sunEdgeProjectedCoords.x /= sunEdgeProjectedCoords.w;
				sunEdgeProjectedCoords.x += 1;
				sunEdgeProjectedCoords.x /= 2;

				sunEdgeProjectedCoords2.x /= sunEdgeProjectedCoords2.w;
				sunEdgeProjectedCoords2.x += 1;
				sunEdgeProjectedCoords2.x /= 2;

				sunSize = std::abs(sunEdgeProjectedCoords.x - sunEdgeProjectedCoords2.x) * 1.6;
				maxSize = std::abs(sunEdgeProjectedCoords.x - sunEdgeProjectedCoords2.x);

				//std::cout << queryDataForSunFlare << "\n";
				//std::cout << "MAX: " << int(maxSize * screenX) * int(maxSize * screenX) * 4 << " ";
				maxSize = int(maxSize * screenX) * int(maxSize * screenX) * 4;
			}
			float sunOcclusionFactor = glm::clamp(queryDataForSunFlare / maxSize, 0.f, 1.f);
			sunOcclusionFactor *= sunOcclusionFactor;

			glm::vec2 center = {0.5,0.5};
			glm::vec2 sunPos = sunProjectedCoords;
			glm::vec2 vectorTwardsCenter = (center - sunPos) * 2.f;

			float globalBrightness = (1 - (glm::length(center - sunPos) / 0.6f)) * sunOcclusionFactor;

			if (globalBrightness > 0)
			{

				glm::vec2 flarePos = sunPos;
				for (int i = 0; i < programData.lensFlare.size(); i++)
				{
					float size = sunSize * screenX;
					//float size = 800;
					flarePos += vectorTwardsCenter / (float)programData.lensFlare.size();

					float textureSize = programData.lensFlare[i].GetSize().x;
					size *= textureSize / programData.maxFlareSize;

					float distanceToCenter = glm::distance(flarePos, center);
					float alpha = 1.f - (distanceToCenter / glm::length(vectorTwardsCenter));
					alpha = glm::clamp(alpha, 0.3f, 1.f) * 0.35;
					alpha *= globalBrightness;

					renderer2d.renderRectangle({
						flarePos.x * screenX - size / 2.f, flarePos.y * screenY - size / 2.f,
						size,size}, programData.lensFlare[i],
						{1,1,1,alpha});

				}

			};



		}


	


		renderer2d.popCamera();
	}


#pragma endregion



	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);
	glBindVertexArray(0);

}

//draw decal
void Renderer::renderDecal(glm::ivec3 position, Camera &c, Block b, ProgramData &programData,
	float crack)
{
	int decalPosition = 0;
	if (crack <= 0) { return; }
	if (crack > 1) { crack = 1; }
	else
	{
		decalPosition = crack * 10;
		if (decalPosition > 10) { decalPosition = 10; }
	}


	//setup geometry
	static std::vector<int> currentVector;
	currentVector.clear();
	
	if (b.isGrassMesh())
	{
		for (int i = 6; i <= 9; i++)
		{
			currentVector.push_back(mergeShorts(i, getGpuIdIndexForBlock(b.getType(), 0)));

			pushFlagsLightAndPosition(currentVector, position, 0, 0, 15, 15, 0);

		}
	}
	else if (b.getType() == BlockTypes::torch)
	{
		for (int i = 0; i < 6; i++)
		{
			currentVector.push_back(mergeShorts(i + 16, getGpuIdIndexForBlock(b.getType(), i)));

			pushFlagsLightAndPosition(currentVector, position, 0, 0, 15, 15, 0);
		}
	}
	else
	{
		bool isAnimated = b.isAnimatedBlock();

		for (int i = 0; i < 6; i++)
		{
			//todo face type or remove
			currentVector.push_back(mergeShorts(i + isAnimated * 10, getGpuIdIndexForBlock(b.getType(), i)));

			pushFlagsLightAndPosition(currentVector, position, 0, 0,
				15, 15, 0);
		}
	}

	
	
	int elementCountSize = currentVector.size() / 4; //todo magic number


#pragma region add data
	{
		glBindBuffer(GL_ARRAY_BUFFER, decalShader.geometry);

		glBufferData(GL_ARRAY_BUFFER, currentVector.size() * sizeof(currentVector[0]),
			currentVector.data(), GL_DYNAMIC_DRAW);
	}
#pragma endregion


	//todo change, also reuse in the the decal shader
	float timeGrass = std::clock() / 1000.f;

	auto viewMatrix = c.getViewMatrix();
	auto vp = c.getProjectionMatrix() * viewMatrix;

	glm::vec3 posFloat = {};
	glm::ivec3 posInt = {};
	glm::ivec3 blockPosition = from3DPointToBlock(c.position);
	c.decomposePosition(posFloat, posInt);

	//setup stuff
	decalShader.shader.bind();
	glUniformMatrix4fv(decalShader.u_viewProjection, 1, GL_FALSE, &vp[0][0]);
	glUniform3fv(decalShader.u_positionFloat, 1, &posFloat[0]);
	glUniform3iv(decalShader.u_positionInt, 1, &posInt[0]);
	glUniform1i(decalShader.u_renderOnlyWater, 0);
	glUniform1f(decalShader.u_timeGrass, timeGrass);
	glUniform1f(decalShader.u_zBias, -0.000001);
	glUniform1i(decalShader.u_crackPosition, decalPosition);
	

	programData.crackTexture.bind(0);


	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	
	glDisable(GL_BLEND);
	decalShader.shader.bind();
	
	glBindVertexArray(decalShader.vao);
	glDrawElementsInstanced(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_BYTE, 0, elementCountSize);


	glBindVertexArray(0);
	glDepthFunc(GL_LESS);
}

void Renderer::renderEntities(
	float deltaTime,
	Camera &c,
	ModelsManager &modelsManager,
	BlocksLoader &blocksLoader, ClientEntityManager &entityManager,
	glm::mat4 &vp, glm::mat4 &projection,
	glm::mat4 &viewMatrix,
	glm::vec3 posFloat,
	glm::ivec3 posInt,
	float exposure, ChunkSystem &chunkSystem, int 
	skyLightIntensity, GLuint64 currentSkinBindlessTexture,
	bool &playerClicked, float playerRunning, BoneTransform &playerHand, int currentHeldItemIndex,
	bool showHand
	)
{

#pragma region setup pipeline

	glBindFramebuffer(GL_FRAMEBUFFER, fboMain.fbo);
	glDepthFunc(GL_LESS);

#pragma endregion


#pragma region other entities setup shader
	
	entityRenderer.basicEntityShader.shader.bind();

	glUniformMatrix4fv(entityRenderer.basicEntityShader.u_viewProjection, 1, GL_FALSE, &vp[0][0]);
	glUniformMatrix4fv(entityRenderer.basicEntityShader.u_view, 1, GL_FALSE, &viewMatrix[0][0]);
	glUniformMatrix4fv(entityRenderer.basicEntityShader.u_modelMatrix, 1, GL_FALSE,
		&glm::mat4(1.f)[0][0]);
	glUniform3fv(entityRenderer.basicEntityShader.u_cameraPositionFloat, 1, &posFloat[0]);
	glUniform3iv(entityRenderer.basicEntityShader.u_cameraPositionInt, 1, &posInt[0]);
	glUniform1f(entityRenderer.basicEntityShader.u_exposure, exposure);
	

#pragma endregion


	struct PerEntityData
	{
		glm::ivec3 entityPositionInt = {};
		glm::vec3 entityPositionFloat = {};
		glm::vec3 color = {1,1,1};
		GLuint64 textureId;
	};

	static std::vector<glm::mat4> skinningMatrix;
	skinningMatrix.clear();

	static std::vector<PerEntityData> entityData;
	entityData.clear();

	glm::mat4 handItemMatrix = glm::mat4{1.f};

	auto renderHand = [&]()
	{
		auto &model = modelsManager.rightHand;

		glBindVertexArray(model.vao);

		glUniform1i(entityRenderer.basicEntityShader.u_bonesPerModel, model.transforms.size());

		skinningMatrix.clear();
		entityData.clear();

		skinningMatrix.reserve(model.transforms.size());
		entityData.reserve(model.transforms.size());

		{
			BoneTransform handIdle;
			handIdle.position = glm::vec3{0.2,-2.0,-0.5};
			handIdle.rotation = glm::vec3{glm::radians(120.f),0.f,0.f};

			if (playerRunning)
			{
				static Oscilator handOscilator(0.3);
				BoneTransform handIdle2;
				handIdle2.position = glm::vec3{0.1,-2.1,-0.4};
				handIdle2.rotation = glm::vec3{glm::radians(123.f),glm::radians(0.f) ,
				glm::radians(2.f)};

				handOscilator.update(deltaTime);

				if (handOscilator.currentFaze)
				{
					handIdle = handIdle2;
				}
			}

			BoneTransform handHit;
			handHit.position = glm::vec3{0.1,-2.1,-0.9};
			handHit.rotation = glm::vec3{glm::radians(90.f),glm::radians(25.f),glm::radians(5.f)};

			float hitSpeed = 6 * deltaTime;
			float returnSpeed = 1 * deltaTime;

			static bool hitReturn = 0;

			if (playerClicked)
			{
				if (hitReturn)
				{
					if (playerHand.goTowards(handIdle, hitSpeed))
					{
						hitReturn = false;
						playerClicked = false;
					}
				}
				else
				{
					if (playerHand.goTowards(handHit, hitSpeed))
					{
						hitReturn = true;
						playerClicked = false;
					}
				}


			}
			else
			{
				hitReturn = 0;
				playerHand.goTowards(handIdle, returnSpeed);
			}



			auto transform = model.transforms[0]; //loaded from glb file
			auto poseMatrix = playerHand.getPoseMatrix();

			glm::vec3 lookDirection = c.viewDirection;
			glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
			glm::vec3 right = glm::normalize(glm::cross(up, lookDirection));
			up = glm::normalize(glm::cross(lookDirection, right));
			auto rotMatrix = glm::mat4(glm::vec4(right, 0.0f), glm::vec4(up, 0.0f), glm::vec4(-lookDirection, 0.0f), glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
			//rotMatrix[0][0] *= -1.0f;

			skinningMatrix.push_back(rotMatrix * glm::scale(glm::vec3{-1,1,1}) * transform * poseMatrix);

			//hand item matrix calculate
			handItemMatrix = skinningMatrix.back() * glm::translate(glm::vec3{0.1,-0.55,-0.4})
				* glm::rotate(glm::radians(180.f), glm::vec3(0, 0, 1))
				* glm::rotate(glm::radians(-45.f), glm::vec3(1, 0, 0))
				* glm::rotate(glm::radians(90.f), glm::vec3(0, 1, 0))
				* glm::scale(glm::vec3(0.45f));
			//handItemMatrix = glm::translate(glm::vec3{0,0,2});

			PerEntityData data = {};
			data.textureId = currentSkinBindlessTexture;

			glm::dvec3 position = glm::dvec3(posInt) + glm::dvec3(posFloat);

			decomposePosition(position, data.entityPositionFloat, data.entityPositionInt);
			entityData.push_back(data);
		}


		if (showHand)
		{
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, skinningMatrixSSBO);
			glBufferData(GL_SHADER_STORAGE_BUFFER, skinningMatrix.size() * sizeof(glm::mat4),
				&skinningMatrix[0][0][0], GL_STREAM_DRAW);

			glBindBuffer(GL_SHADER_STORAGE_BUFFER, perEntityDataSSBO);
			glBufferData(GL_SHADER_STORAGE_BUFFER, entityData.size() * sizeof(entityData[0]),
				entityData.data(), GL_STREAM_DRAW);

			glDrawElements(GL_TRIANGLES, model.vertexCount, GL_UNSIGNED_INT, nullptr);
		};

	};

	//render hand draw hand renderHand drawHand
	renderHand();



	auto renderAllEntitiesOfOneType = [&](Model &model, auto &container)
	{

		glBindVertexArray(model.vao);

		glUniform1i(entityRenderer.basicEntityShader.u_bonesPerModel, model.transforms.size());

		skinningMatrix.clear();
		entityData.clear();

		skinningMatrix.reserve(model.transforms.size() * container.size());
		entityData.reserve(model.transforms.size() * container.size());

		for (auto &e : container)
		{
			PerEntityData data = {};

			auto rotMatrix = e.second.getBodyRotationMatrix();

			if constexpr (hasCanBeKilled<decltype(e.second.entity)>)
			{
				if (e.second.wasKilled)
				{
					data.color = {0.6,0.2,0.2};
					rotMatrix = rotMatrix * glm::rotate(PI / 2.f, glm::vec3{0,0,1});

					if (e.second.wasKilledTimer <= 0)
					{
						continue;
					}
				}
			}

			for (int i = 0; i < model.transforms.size(); i++)
			{
				skinningMatrix.push_back(rotMatrix * model.transforms[i]);
			}

			//todo set kill animation or something

			e.second.setEntityMatrix(skinningMatrix.data() + (skinningMatrix.size() - model.transforms.size()));


			if constexpr (hasSkinBindlessTexture<decltype(e.second)>)
			{
				if (e.second.skinBindlessTexture)
				{
					data.textureId = e.second.skinBindlessTexture;
				}
				else
				{
					data.textureId = modelsManager.gpuIds[e.second.getTextureIndex()];
				}

			}
			else
			{
				data.textureId = modelsManager.gpuIds[e.second.getTextureIndex()];
			}

			decomposePosition(e.second.getRubberBandPosition(), data.entityPositionFloat, data.entityPositionInt);
			entityData.push_back(data);
		}

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, skinningMatrixSSBO);
		//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, skinningMatrixSSBO);
		glBufferData(GL_SHADER_STORAGE_BUFFER, skinningMatrix.size() * sizeof(glm::mat4),
			&skinningMatrix[0][0][0], GL_STREAM_DRAW);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, perEntityDataSSBO);
		//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, perEntityDataSSBO);
		glBufferData(GL_SHADER_STORAGE_BUFFER, entityData.size() * sizeof(entityData[0]),
			entityData.data(), GL_STREAM_DRAW);

		glDrawElementsInstanced(GL_TRIANGLES, model.vertexCount, GL_UNSIGNED_INT, nullptr,
			container.size());

	};

	//todo remove
	entityRenderer.itemEntitiesToRender.clear();

	renderAllEntitiesOfOneType(modelsManager.human, entityManager.players);
	renderAllEntitiesOfOneType(modelsManager.human, entityManager.zombies);
	renderAllEntitiesOfOneType(modelsManager.pig, entityManager.pigs);
	renderAllEntitiesOfOneType(modelsManager.cat, entityManager.cats);
	renderAllEntitiesOfOneType(modelsManager.goblin, entityManager.goblins);


	glBindVertexArray(0);


#pragma region cubeEntities
	{

		entityRenderer.blockEntityshader.shader.bind();

		glBindVertexArray(entityRenderer.blockEntityshader.vaoCube);


		glUniformMatrix4fv(entityRenderer.blockEntityshader.u_viewProjection, 1, GL_FALSE, &vp[0][0]);
		glUniformMatrix4fv(entityRenderer.blockEntityshader.u_view, 1, GL_FALSE, &viewMatrix[0][0]);
		glUniformMatrix4fv(entityRenderer.blockEntityshader.u_modelMatrix, 1, GL_FALSE,
			glm::value_ptr(glm::scale(glm::vec3{0.4f})));
		glUniform3fv(entityRenderer.blockEntityshader.u_cameraPositionFloat, 1, &posFloat[0]);
		glUniform3iv(entityRenderer.blockEntityshader.u_cameraPositionInt, 1, &posInt[0]);

		//debug stuff
		if (0)
		{
			//todo something better here lol
			std::uint64_t textures[6] = {};

			for (int i = 0; i < 6; i++)
			{
				textures[i] = blocksLoader.gpuIds[getGpuIdIndexForBlock(BlockTypes::bookShelf, i)];
			}

			glUniformHandleui64vARB(entityRenderer.blockEntityshader.u_texture, 6, textures);


			//todo instance rendering
			for (auto &e : entityRenderer.itemEntitiesToRender)
			{
				glm::vec3 entityFloat = {};
				glm::ivec3 entityInt = {};

				decomposePosition(e.position, entityFloat, entityInt);

				entityFloat += glm::vec3(0, 0.2, 0);

				glUniform3fv(entityRenderer.blockEntityshader.u_entityPositionFloat, 1, &entityFloat[0]);
				glUniform3iv(entityRenderer.blockEntityshader.u_entityPositionInt, 1, &entityInt[0]);

				glDrawArrays(GL_TRIANGLES, 0, 36);

			}

		}

		//real entities
		{

			//todo instance rendering
			for (auto &e : entityManager.droppedItems)
			{
				if (!isBlock(e.second.entity.type)) { continue; }

				//todo something better here lol
				std::uint64_t textures[6] = {};

				if (e.second.entity.type >= ItemsStartPoint)
				{
					for (int i = 0; i < 6; i++)
					{
						textures[i] = blocksLoader.gpuIdsItems
							[e.second.entity.type - ItemsStartPoint];
					}
				}
				else
				{
					for (int i = 0; i < 6; i++)
					{
						textures[i] = blocksLoader.gpuIds
							[getGpuIdIndexForBlock(e.second.entity.type, i)];
					}
				}


				glUniformHandleui64vARB(entityRenderer.blockEntityshader.u_texture, 6, textures);

				glm::vec3 entityFloat = {};
				glm::ivec3 entityInt = {};

				//decomposePosition(e.second.item.position, entityFloat, entityInt);
				decomposePosition(e.second.getRubberBandPosition(), entityFloat, entityInt);

				entityFloat += glm::vec3(0, 0.2, 0);

				glUniform3fv(entityRenderer.blockEntityshader.u_entityPositionFloat, 1, &entityFloat[0]);
				glUniform3iv(entityRenderer.blockEntityshader.u_entityPositionInt, 1, &entityInt[0]);


				auto b = chunkSystem.getBlockSafe(e.second.getRubberBandPosition() + glm::dvec3(0, 0.2, 0));
				int rez = skyLightIntensity;

				if (b)
				{
					rez = std::max((char)b->getLight(), (char)(b->getSkyLight() - ((char)15 - (char)skyLightIntensity)));
					rez = std::max(rez, 0);
				}

				if (dontUpdateLightSystem)
				{
					rez = 15;
				}

				glUniform1i(entityRenderer.blockEntityshader.u_lightValue, rez);


				glDrawArrays(GL_TRIANGLES, 0, 36);
			}

		}


	}
#pragma endregion


#pragma region item entities

	entityRenderer.itemEntityShader.shader.bind();

	glUniformMatrix4fv(entityRenderer.itemEntityShader.u_viewProjection, 1, GL_FALSE, &vp[0][0]);
	glUniformMatrix4fv(entityRenderer.itemEntityShader.u_view, 1, GL_FALSE, &viewMatrix[0][0]);
	glUniformMatrix4fv(entityRenderer.itemEntityShader.u_modelMatrix, 1, GL_FALSE,
		glm::value_ptr(glm::scale(glm::vec3{0.4f})));
	glUniform3fv(entityRenderer.itemEntityShader.u_cameraPositionFloat, 1, &posFloat[0]);
	glUniform3iv(entityRenderer.itemEntityShader.u_cameraPositionInt, 1, &posInt[0]);

	{

		auto renderOneItem = [&](auto type, glm::dvec3 pos, int light, glm::mat4 *mat = 0)
		{
			std::uint64_t texture;

			BlocksLoader::ItemGeometry geometry = {};

			if (type >= ItemsStartPoint)
			{
				texture = blocksLoader.gpuIdsItems
					[type - ItemsStartPoint];

				geometry = blocksLoader.itemsGeometry[type - ItemsStartPoint + 1];
			}
			else
			{
				texture = blocksLoader.gpuIds
					[getGpuIdIndexForBlock(type, 0)];
			}

			glUniformHandleui64ARB(entityRenderer.itemEntityShader.u_texture, texture);
			glUniform1i(entityRenderer.itemEntityShader.u_lightValue, light);

			if (mat)
			{
				glm::vec3 entityFloat = {};
				glm::ivec3 entityInt = {};

				decomposePosition(c.position, entityFloat, entityInt);

				glUniform3fv(entityRenderer.itemEntityShader.u_entityPositionFloat, 1, &entityFloat[0]);
				glUniform3iv(entityRenderer.itemEntityShader.u_entityPositionInt, 1, &entityInt[0]);

				glUniformMatrix4fv(entityRenderer.itemEntityShader.u_modelMatrix, 1, GL_FALSE,
					glm::value_ptr(*mat));
			}
			else
			{
				glm::vec3 entityFloat = {};
				glm::ivec3 entityInt = {};

				decomposePosition(pos, entityFloat, entityInt);

				entityFloat += glm::vec3(0, 0.4, 0);

				glUniform3fv(entityRenderer.itemEntityShader.u_entityPositionFloat, 1, &entityFloat[0]);
				glUniform3iv(entityRenderer.itemEntityShader.u_entityPositionInt, 1, &entityInt[0]);

				glUniformMatrix4fv(entityRenderer.itemEntityShader.u_modelMatrix, 1, GL_FALSE,
					glm::value_ptr(glm::scale(glm::vec3{0.4f})));
			}


			glBindVertexArray(geometry.vao);
			glDrawArrays(GL_TRIANGLES, 0, geometry.count);
		
		};

		//todo instance rendering
		for (auto &e : entityManager.droppedItems)
		{
			//continue;
			if (isBlock(e.second.entity.type)) { continue; }
			if (!e.second.entity.type) { continue; }

			auto b = chunkSystem.getBlockSafe(e.second.getRubberBandPosition() + glm::dvec3(0, 0.2, 0));
			int rez = skyLightIntensity;

			if (b)
			{
				rez = std::max((char)b->getLight(), (char)(b->getSkyLight() - ((char)15 - (char)skyLightIntensity)));
				rez = std::max(rez, 0);
			}

			if (dontUpdateLightSystem)
			{
				rez = 15;
			}

			renderOneItem(e.second.entity.type, e.second.getRubberBandPosition(), rez);

		}


		//hand item
		auto handItem = entityManager.localPlayer.inventory.getItemFromIndex(currentHeldItemIndex);
		if(handItem)	
		{

			if (handItem->isBlock())
			{


			}
			else if(handItem->type)
			{
				renderOneItem(handItem->type, {}, 15, &handItemMatrix);
			}


		}


	}


#pragma endregion



}


//https://www.youtube.com/watch?v=lUo7s-i9Gy4&ab_channel=OREONENGINE
void computeFrustumDimensions(
	glm::vec3 position, glm::vec3 viewDirection,
	float fovRadians, float aspectRatio, float nearPlane, float farPlane,
	glm::vec2 &nearDimensions, glm::vec2 &farDimensions, glm::vec3 &centerNear, glm::vec3 &centerFar)
{
	float tanFov2 = tan(fovRadians) * 2;

	nearDimensions.y = tanFov2 * nearPlane;			//hNear
	nearDimensions.x = nearDimensions.y * aspectRatio;	//wNear

	farDimensions.y = tanFov2 * farPlane;			//hNear
	farDimensions.x = farDimensions.y * aspectRatio;	//wNear

	centerNear = position + viewDirection * farPlane;
	centerFar = position + viewDirection * farPlane;

}

void generateTangentSpace(glm::vec3 v, glm::vec3 &outUp, glm::vec3 &outRight)
{
	glm::vec3 up(0, 1, 0);
	if (v == up)
	{
		outRight = glm::vec3(1, 0, 0);
	}
	else
	{
		outRight = normalize(glm::cross(v, up));
	}
	outUp = normalize(cross(outRight, v));
}

//https://www.youtube.com/watch?v=lUo7s-i9Gy4&ab_channel=OREONENGINE
void computeFrustumSplitCorners(glm::vec3 directionVector,
	glm::vec2 nearDimensions, glm::vec2 farDimensions, glm::vec3 centerNear, glm::vec3 centerFar,
	glm::vec3 &nearTopLeft, glm::vec3 &nearTopRight, glm::vec3 &nearBottomLeft, glm::vec3 &nearBottomRight,
	glm::vec3 &farTopLeft, glm::vec3 &farTopRight, glm::vec3 &farBottomLeft, glm::vec3 &farBottomRight)
{

	glm::vec3 rVector = {};
	glm::vec3 upVectpr = {};

	generateTangentSpace(directionVector, upVectpr, rVector);

	nearTopLeft = centerNear + upVectpr * nearDimensions.y / 2.f - rVector * nearDimensions.x / 2.f;
	nearTopRight = centerNear + upVectpr * nearDimensions.y / 2.f + rVector * nearDimensions.x / 2.f;
	nearBottomLeft = centerNear - upVectpr * nearDimensions.y / 2.f - rVector * nearDimensions.x / 2.f;
	nearBottomRight = centerNear - upVectpr * nearDimensions.y / 2.f + rVector * nearDimensions.x / 2.f;

	farTopLeft = centerNear + upVectpr * farDimensions.y / 2.f - rVector * farDimensions.x / 2.f;
	farTopRight = centerNear + upVectpr * farDimensions.y / 2.f + rVector * farDimensions.x / 2.f;
	farBottomLeft = centerNear - upVectpr * farDimensions.y / 2.f - rVector * farDimensions.x / 2.f;
	farBottomRight = centerNear - upVectpr * farDimensions.y / 2.f + rVector * farDimensions.x / 2.f;

}

glm::mat4 calculateLightProjectionMatrix(Camera &camera, glm::vec3 lightDir,
	float nearPlane, float farPlane,
	float zOffset)
{
	glm::vec3 direction = lightDir;
	glm::vec3 eye = camera.position - glm::dvec3(direction);
	glm::vec3 center = eye + direction;

	glm::mat4 lightView = lookAtSafe(eye, center, {0.f,1.f,0.f});//todo option to add a custom vector direction

	glm::vec3 rVector = {};
	glm::vec3 upVectpr = {};
	generateTangentSpace(lightDir, upVectpr, rVector);

	glm::vec2 nearDimensions{};
	glm::vec2 farDimensions{};
	glm::vec3 centerNear{};
	glm::vec3 centerFar{};

	computeFrustumDimensions(camera.position, camera.viewDirection, camera.fovRadians, camera.aspectRatio,
		nearPlane, farPlane, nearDimensions, farDimensions, centerNear, centerFar);

	glm::vec3 nearTopLeft{};
	glm::vec3 nearTopRight{};
	glm::vec3 nearBottomLeft{};
	glm::vec3 nearBottomRight{};
	glm::vec3 farTopLeft{};
	glm::vec3 farTopRight{};
	glm::vec3 farBottomLeft{};
	glm::vec3 farBottomRight{};

	computeFrustumSplitCorners(camera.viewDirection, nearDimensions, farDimensions, centerNear, centerFar,
		nearTopLeft,
		nearTopRight,
		nearBottomLeft,
		nearBottomRight,
		farTopLeft,
		farTopRight,
		farBottomLeft,
		farBottomRight
	);


	glm::vec3 corners[] =
	{
		nearTopLeft,
		nearTopRight,
		nearBottomLeft,
		nearBottomRight,
		farTopLeft,
		farTopRight,
		farBottomLeft,
		farBottomRight,
	};

	float longestDiagonal = glm::distance(nearTopLeft, farBottomRight);

	glm::vec3 minVal{};
	glm::vec3 maxVal{};

	for (int i = 0; i < 8; i++)
	{
		glm::vec4 corner(corners[i], 1);

		glm::vec4 lightViewCorner = lightView * corner;

		if (i == 0)
		{
			minVal = lightViewCorner;
			maxVal = lightViewCorner;
		}
		else
		{
			if (lightViewCorner.x < minVal.x) { minVal.x = lightViewCorner.x; }
			if (lightViewCorner.y < minVal.y) { minVal.y = lightViewCorner.y; }
			if (lightViewCorner.z < minVal.z) { minVal.z = lightViewCorner.z; }

			if (lightViewCorner.x > maxVal.x) { maxVal.x = lightViewCorner.x; }
			if (lightViewCorner.y > maxVal.y) { maxVal.y = lightViewCorner.y; }
			if (lightViewCorner.z > maxVal.z) { maxVal.z = lightViewCorner.z; }

		}

	}

	//keep them square and the same size:
	//https://www.youtube.com/watch?v=u0pk1LyLKYQ&t=99s&ab_channel=WesleyLaFerriere
	if (1)
	{
		float firstSize = maxVal.x - minVal.x;
		float secondSize = maxVal.y - minVal.y;
		float thirdSize = maxVal.z - minVal.z;

		{
			float ratio = longestDiagonal / firstSize;

			glm::vec2 newVecValues = {minVal.x, maxVal.x};
			float dimension = firstSize;
			float dimensionOver2 = dimension / 2.f;

			newVecValues -= glm::vec2(minVal.x + dimensionOver2, minVal.x + dimensionOver2);
			newVecValues *= ratio;
			newVecValues += glm::vec2(minVal.x + dimensionOver2, minVal.x + dimensionOver2);

			minVal.x = newVecValues.x;
			maxVal.x = newVecValues.y;
		}

		{
			float ratio = longestDiagonal / secondSize;

			glm::vec2 newVecValues = {minVal.y, maxVal.y};
			float dimension = secondSize;
			float dimensionOver2 = dimension / 2.f;

			newVecValues -= glm::vec2(minVal.y + dimensionOver2, minVal.y + dimensionOver2);
			newVecValues *= ratio;
			newVecValues += glm::vec2(minVal.y + dimensionOver2, minVal.y + dimensionOver2);

			minVal.y = newVecValues.x;
			maxVal.y = newVecValues.y;
		}

		{//this size probably can be far-close
			float ratio = longestDiagonal / thirdSize;

			glm::vec2 newVecValues = {minVal.z, maxVal.z};
			float dimension = thirdSize;
			float dimensionOver2 = dimension / 2.f;

			newVecValues -= glm::vec2(minVal.z + dimensionOver2, minVal.z + dimensionOver2);
			newVecValues *= ratio;
			newVecValues += glm::vec2(minVal.z + dimensionOver2, minVal.z + dimensionOver2);

			minVal.z = newVecValues.x;
			maxVal.z = newVecValues.y;
		}

	}

	float near_plane = minVal.z - zOffset;
	float far_plane = maxVal.z;


	glm::vec2 ortoMin = {minVal.x, minVal.y};
	glm::vec2 ortoMax = {maxVal.x, maxVal.y};

	//remove shadow flicker
	if (1)
	{
		glm::vec2 shadowMapSize(2048, 2048);
		glm::vec2 worldUnitsPerTexel = (ortoMax - ortoMin) / shadowMapSize;

		ortoMin /= worldUnitsPerTexel;
		ortoMin = glm::floor(ortoMin);
		ortoMin *= worldUnitsPerTexel;

		ortoMax /= worldUnitsPerTexel;
		ortoMax = glm::floor(ortoMax);
		ortoMax *= worldUnitsPerTexel;

		float zWorldUnitsPerTexel = (far_plane - near_plane) / 2048;

		near_plane /= zWorldUnitsPerTexel;
		far_plane /= zWorldUnitsPerTexel;
		near_plane = glm::floor(near_plane);
		far_plane = glm::floor(far_plane);
		near_plane *= zWorldUnitsPerTexel;
		far_plane *= zWorldUnitsPerTexel;

	}

	glm::mat4 lightProjection = glm::ortho(ortoMin.x, ortoMax.x, ortoMin.y, ortoMax.y, near_plane, far_plane);

	return lightProjection * lightView;

};

glm::quat quatFromDirection(const glm::vec3 &direction, const glm::vec3 &referenceForward = glm::vec3(0, 0, -1))
{
	// Normalize the direction vector
	glm::vec3 normalizedDir = glm::normalize(direction);

	// Compute the angle between the reference forward vector and the direction vector
	float angle = acos(glm::clamp(glm::dot(referenceForward, normalizedDir), -1.0f, 1.0f));

	// Compute the rotation axis using the cross product of reference forward and target direction
	glm::vec3 rotationAxis = glm::cross(referenceForward, normalizedDir);

	// If the direction is nearly the same as the forward vector, return identity quaternion
	if (glm::length(rotationAxis) < 0.0001f)
	{
		return glm::quat(1, 0, 0, 0); // Identity quaternion
	}

	// Normalize the rotation axis
	rotationAxis = glm::normalize(rotationAxis);

	// Create the quaternion from the axis and angle
	return glm::angleAxis(angle, rotationAxis);
}

//todo
//https://github.com/maritim/LiteEngine/blob/8e165cb06e1dccf378cde0e34f17668e6d08c15b/Engine/RenderPasses/ShadowMap/DirectionalLightShadowMapRenderPass.cpp#L423
glm::mat4x4 getLightMatrix(glm::vec3 lightAngle, Camera &c)
{

	const float LIGHT_CAMERA_OFFSET = 100.0f;

	glm::quat lightRotation = glm::conjugate(quatFromDirection(lightAngle));

	//glm::mat4 cameraView = glm::translate(glm::mat4_cast(viewCamera->GetRotation()), viewCamera->GetPosition() * -1.0f);
	glm::mat4 cameraView = c.getViewMatrix();
	glm::mat4 cameraProjection = c.getProjectionMatrix();
	glm::mat4 invCameraProjView = glm::inverse(cameraProjection * cameraView);

	//RenderLightObject::Shadow shadow = renderLightObject->GetShadow();

	//for (std::size_t index = 0; index < shadow.cascadesCount; index++)
	{

		//OrthographicCamera *lightCamera = (OrthographicCamera *)_volume->GetLightCamera(index);

		glm::vec3 cuboidExtendsMin = glm::vec3(std::numeric_limits<float>::max());
		glm::vec3 cuboidExtendsMax = glm::vec3(-std::numeric_limits<float>::min());

		//float zStart = index == 0 ? -1 : _volume->GetCameraLimit(index - 1);
		//float zEnd = _volume->GetCameraLimit(index);

		float zStart = -1;
		float zEnd = -0.9;

		for (int x = -1; x <= 1; x += 2)
		{
			for (int y = -1; y <= 1; y += 2)
			{
				for (int z = -1; z <= 1; z += 2)
				{
					glm::vec4 cuboidCorner = glm::vec4(x, y, z == -1 ? zStart : zEnd, 1.0f);

					cuboidCorner = invCameraProjView * cuboidCorner;
					cuboidCorner /= cuboidCorner.w;

					cuboidCorner = glm::vec4(lightRotation * glm::vec3(cuboidCorner), 0.0f);

					cuboidExtendsMin.x = std::min(cuboidExtendsMin.x, cuboidCorner.x);
					cuboidExtendsMin.y = std::min(cuboidExtendsMin.y, cuboidCorner.y);
					cuboidExtendsMin.z = std::min(cuboidExtendsMin.z, cuboidCorner.z);

					cuboidExtendsMax.x = std::max(cuboidExtendsMax.x, cuboidCorner.x);
					cuboidExtendsMax.y = std::max(cuboidExtendsMax.y, cuboidCorner.y);
					cuboidExtendsMax.z = std::max(cuboidExtendsMax.z, cuboidCorner.z);
				}
			}
		}

		//lightCamera->SetRotation(lightRotation);

		//lightCamera->SetOrthographicInfo(
		//	cuboidExtendsMin.x, cuboidExtendsMax.x,
		//	cuboidExtendsMin.y, cuboidExtendsMax.y,
		//	cuboidExtendsMin.z - LIGHT_CAMERA_OFFSET, cuboidExtendsMax.z + LIGHT_CAMERA_OFFSET
		//);

		//_volume->SetLightCamera(index, lightCamera);

		glm::mat4 lightProjection = glm::ortho(cuboidExtendsMin.x, cuboidExtendsMax.x,
			cuboidExtendsMin.y, cuboidExtendsMax.y,
			cuboidExtendsMin.z - LIGHT_CAMERA_OFFSET, cuboidExtendsMax.z + LIGHT_CAMERA_OFFSET);

		return lightProjection;

	}
}


void Renderer::renderShadow(SunShadow &sunShadow,
	ChunkSystem &chunkSystem, Camera &c, ProgramData &programData, glm::vec3 sunPos)
{
	glEnable(GL_DEPTH_TEST);
	glColorMask(0, 0, 0, 0);

	glViewport(0, 0, 2048, 2048);
	glBindFramebuffer(GL_FRAMEBUFFER, sunShadow.shadowMap.fbo);
	glClear(GL_DEPTH_BUFFER_BIT);


	glm::ivec3 newPos = c.position;

	{
		//newPos.y = 120;
		newPos.y += 60;

		glm::vec3 moveDir = sunPos;

		float l = glm::length(moveDir);
		if (l > 0.0001)
		{
			moveDir /= l;
			newPos += moveDir * (float)CHUNK_SIZE * 12.f;
		}
	}

	//glm::vec3 posFloat = {};
	//glm::ivec3 posInt = {};
	//c.decomposePosition(posFloat, posInt);

	glm::vec3 posFloat = {};
	glm::ivec3 posInt = newPos;
	
	glm::ivec3 playerPos = c.position;

	glm::vec3 vectorToPlayer = glm::normalize(glm::vec3(playerPos - newPos));

	//posInt += glm::vec3(0, 0, 10);

	float projectSize = 60;

	float near_plane = 1.0f, far_plane = 460.f;
	glm::mat4 lightProjection = glm::ortho(-projectSize, projectSize, -projectSize, projectSize,
		near_plane,		far_plane);
	//auto mvp = lightProjection * glm::lookAt({},
	//	-skyBoxRenderer.sunPos, glm::vec3(0, 1, 0));
	auto mvp = lightProjection * lookAtSafe({},
		vectorToPlayer, glm::vec3(0, 1, 0));

	//glm::mat4 mvp = getLightMatrix(sunPos, c);
	

	//auto mat = calculateLightProjectionMatrix(c, -skyBoxRenderer.sunPos, 1, 260, 25);

	sunShadow.lightSpaceMatrix = mvp;
	sunShadow.lightSpacePosition = posInt;
	
	//sunShadow.lightSpaceMatrix = mat;
	//sunShadow.lightSpacePosition = {};

#pragma region setup uniforms and stuff
	{
		zpassShader.shader.bind();
		glUniformMatrix4fv(zpassShader.u_viewProjection, 1, GL_FALSE, &sunShadow.lightSpaceMatrix[0][0]);
		glUniform3fv(zpassShader.u_positionFloat, 1, &posFloat[0]);
		glUniform3iv(zpassShader.u_positionInt, 1, &sunShadow.lightSpacePosition[0]);
		glUniform1i(zpassShader.u_renderOnlyWater, 0);
	}
#pragma endregion




	for (auto &chunk : chunkSystem.loadedChunks)
	{
		if (chunk)
		{
			if (!chunk->isDontDrawYet())
			{
				int facesCount = chunk->elementCountSize;
				if (facesCount)
				{
					glBindVertexArray(chunk->vao);
					glDrawElementsInstanced(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_BYTE, 0, facesCount);
				}
			}
		}
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glColorMask(1, 1, 1, 1);

}

float cubePositions[] = {
		-0.5f, +0.5f, +0.5f, // 0
		+0.5f, +0.5f, +0.5f, // 1
		+0.5f, +0.5f, -0.5f, // 2
		-0.5f, +0.5f, -0.5f, // 3
		-0.5f, +0.5f, -0.5f, // 4
		+0.5f, +0.5f, -0.5f, // 5
		+0.5f, -0.5f, -0.5f, // 6
		-0.5f, -0.5f, -0.5f, // 7
		+0.5f, +0.5f, -0.5f, // 8
		+0.5f, +0.5f, +0.5f, // 9
		+0.5f, -0.5f, +0.5f, // 10
		+0.5f, -0.5f, -0.5f, // 11
		-0.5f, +0.5f, +0.5f, // 12
		-0.5f, +0.5f, -0.5f, // 13
		-0.5f, -0.5f, -0.5f, // 14
		-0.5f, -0.5f, +0.5f, // 15
		+0.5f, +0.5f, +0.5f, // 16
		-0.5f, +0.5f, +0.5f, // 17
		-0.5f, -0.5f, +0.5f, // 18
		+0.5f, -0.5f, +0.5f, // 19
		+0.5f, -0.5f, -0.5f, // 20
		-0.5f, -0.5f, -0.5f, // 21
		-0.5f, -0.5f, +0.5f, // 22
		+0.5f, -0.5f, +0.5f, // 23
};

unsigned int cubeIndicesData[] = {
	0,   1,  2,  0,  2,  3, // Top
	4,   5,  6,  4,  6,  7, // Back
	8,   9, 10,  8, 10, 11, // Right
	12, 13, 14, 12, 14, 15, // Left
	16, 17, 18, 16, 18, 19, // Front
	20, 22, 21, 20, 23, 22, // Bottom
};

void GyzmosRenderer::create()
{


	gyzmosLineShader.loadShaderProgramFromFile(RESOURCES_PATH "shaders/rendering/gyzmosLinesShader.vert",
		RESOURCES_PATH "shaders/rendering/gyzmosCubeShader.frag");
	GET_UNIFORM(gyzmosLineShader, u_gyzmosLineShaderViewProjection);


	gyzmosCubeShader.loadShaderProgramFromFile(RESOURCES_PATH "shaders/rendering/gyzmosCubeShader.vert",
		RESOURCES_PATH "shaders/rendering/gyzmosCubeShader.frag");

	GET_UNIFORM(gyzmosCubeShader, u_viewProjection);
	GET_UNIFORM(gyzmosCubeShader, u_positionInt);
	GET_UNIFORM(gyzmosCubeShader, u_positionFloat);
	

	glCreateVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glCreateBuffers(1, &vertexDataBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexDataBuffer);
	glBufferStorage(GL_ARRAY_BUFFER, sizeof(cubePositions), cubePositions, 0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
	glVertexAttribDivisor(0, 0);

	glCreateBuffers(1, &blockPositionBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, blockPositionBuffer);
	glBufferData(GL_ARRAY_BUFFER, 0, 0, GL_STREAM_DRAW);
	glVertexAttribIPointer(1, 3, GL_INT, 0, 0);
	glEnableVertexAttribArray(1);
	glVertexAttribDivisor(1, 1);

	glCreateBuffers(1, &cubeIndices);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeIndices);
	glBufferStorage(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIndicesData), (void*)cubeIndicesData, 0);



	glCreateVertexArrays(1, &vaoLines);
	glBindVertexArray(vaoLines);

	glCreateBuffers(1, &vertexDataBufferLines);
	glBindBuffer(GL_ARRAY_BUFFER, vertexDataBufferLines);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);

}


void GyzmosRenderer::render(Camera &c, glm::ivec3 posInt, glm::vec3 posFloat)
{


	if (!cubes.empty()) 
	{
		auto mvp = c.getProjectionMatrix() * glm::lookAt({0,0,0}, c.viewDirection, c.up);

		glDisable(GL_CULL_FACE);

		glNamedBufferData(blockPositionBuffer, cubes.size() * sizeof(CubeData), cubes.data(), GL_STATIC_DRAW);

		gyzmosCubeShader.bind();

		glDepthFunc(GL_LEQUAL);
		glBindVertexArray(vao);

		glUniformMatrix4fv(u_viewProjection, 1, GL_FALSE, &mvp[0][0]);
		glUniform3fv(u_positionFloat, 1, &posFloat[0]);
		glUniform3iv(u_positionInt, 1, &posInt[0]);

		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glDrawElementsInstanced(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0, cubes.size());
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		glBindVertexArray(0);
		glDepthFunc(GL_LESS);

		glEnable(GL_CULL_FACE);
	
		cubes.clear();

	}

	if (!lines.empty())
	{
		glLineWidth(10);

		auto mvp = c.getProjectionMatrix() * 
			glm::lookAt( glm::vec3(c.position), glm::vec3(c.position)
			+ c.viewDirection, c.up);


		glNamedBufferData(vertexDataBufferLines, lines.size() * 
			sizeof(LinesData), lines.data(), GL_STREAM_DRAW);

		gyzmosLineShader.bind();

		glDepthFunc(GL_LEQUAL);
		glBindVertexArray(vaoLines);


		glUniformMatrix4fv(u_gyzmosLineShaderViewProjection, 1, GL_FALSE, &mvp[0][0]);

		glDrawArrays(GL_LINES, 0, lines.size()*2);

		glBindVertexArray(0);
		glDepthFunc(GL_LESS);


		lines.clear();
	}

}



void PointDebugRenderer::create()
{
	pointDebugShader.loadShaderProgramFromFile(RESOURCES_PATH "shaders/rendering/pointDebugShader.vert",
		RESOURCES_PATH "shaders/rendering/pointDebugShader.frag");

	GET_UNIFORM(pointDebugShader, u_viewProjection);
	GET_UNIFORM(pointDebugShader, u_positionInt);
	GET_UNIFORM(pointDebugShader, u_positionFloat);
	GET_UNIFORM(pointDebugShader, u_blockPositionInt);
	GET_UNIFORM(pointDebugShader, u_blockPositionFloat);


	
}

void PointDebugRenderer::renderPoint(Camera &c, glm::dvec3 point)
{
	pointDebugShader.bind();

	auto mvp = c.getProjectionMatrix() * glm::lookAt({0,0,0}, c.viewDirection, c.up);

	glm::vec3 posFloat = {};
	glm::ivec3 posInt = {};
	c.decomposePosition(posFloat, posInt);

	glUniformMatrix4fv(u_viewProjection, 1, GL_FALSE, &mvp[0][0]);
	glUniform3fv(u_positionFloat, 1, &posFloat[0]);
	glUniform3iv(u_positionInt, 1, &posInt[0]);

	glm::vec3 posFloatBlock = {};
	glm::ivec3 posIntBlock = {};
	decomposePosition(point, posFloatBlock, posIntBlock);

	glUniform3fv(u_blockPositionFloat, 1, &posFloatBlock[0]);
	glUniform3iv(u_blockPositionInt, 1, &posIntBlock[0]);

	glPointSize(15);
	glDrawArrays(GL_POINTS, 0, 1);

}

void PointDebugRenderer::renderCubePoint(Camera &c, glm::dvec3 point)
{
	renderPoint(c, point);
	
	renderPoint(c, point + glm::dvec3(0.5,0.5,-0.5));
	renderPoint(c, point + glm::dvec3(0.5,0.5,0.5));
	renderPoint(c, point + glm::dvec3(-0.5,0.5,-0.5));
	renderPoint(c, point + glm::dvec3(-0.5,0.5,0.5));

	renderPoint(c, point + glm::dvec3(0.5, -0.5, -0.5));
	renderPoint(c, point + glm::dvec3(0.5, -0.5, 0.5));
	renderPoint(c, point + glm::dvec3(-0.5, -0.5, -0.5));
	renderPoint(c, point + glm::dvec3(-0.5, -0.5, 0.5));

}


#undef GET_UNIFORM

void Renderer::FBO::create(GLint addColor, bool addDepth,
	GLint addSecondaryRenderTarget, GLint addThirdRenderTarget, GLint addFourthRenderTarget)
{
	if (addColor == 1) { addColor = GL_RGBA8; }
	if (addSecondaryRenderTarget == 1) { addSecondaryRenderTarget = GL_RGBA8; }
	if (addThirdRenderTarget == 1) { addThirdRenderTarget = GL_RGBA8; }
	if (addFourthRenderTarget == 1) { addFourthRenderTarget = GL_RGBA8; }

	colorFormat = addColor;
	secondaryColorFormat = addSecondaryRenderTarget;
	thirdColorFormat = addThirdRenderTarget;
	fourthColorFormat = addFourthRenderTarget;

	permaAssert(!(addColor == 0 && addSecondaryRenderTarget != 0));
	permaAssert(!(addSecondaryRenderTarget == 0 && addThirdRenderTarget != 0));


	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);


	if (addColor)
	{
		glGenTextures(1, &color);
		glBindTexture(GL_TEXTURE_2D, color);
		glTexImage2D(GL_TEXTURE_2D, 0, colorFormat, 1, 1
			, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color, 0);
	}

	if (addSecondaryRenderTarget)
	{
		glGenTextures(1, &secondaryColor);
		glBindTexture(GL_TEXTURE_2D, secondaryColor);
		// Set the width and height of your texture (e.g., 1024x1024)
		glTexImage2D(GL_TEXTURE_2D, 0, secondaryColorFormat, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		// Attach the secondary color texture to the framebuffer
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, secondaryColor, 0);

		unsigned int attachments[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
		glDrawBuffers(2, attachments);
	}

	if (addThirdRenderTarget)
	{
		glGenTextures(1, &thirdColor);
		glBindTexture(GL_TEXTURE_2D, thirdColor);
		
		GLenum internalFormat = GL_RGB;
		if (thirdColorFormat == GL_RGB16UI) { internalFormat = GL_RGB_INTEGER; }

		glTexImage2D(GL_TEXTURE_2D, 0, thirdColorFormat, 1, 1, 0, internalFormat, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, secondaryColor, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, thirdColor, 0);

		unsigned int attachments[3] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1,
			GL_COLOR_ATTACHMENT2};
		glDrawBuffers(3, attachments);
	}
	
	if (addFourthRenderTarget)
	{
		glGenTextures(1, &fourthColor);
		glBindTexture(GL_TEXTURE_2D, fourthColor);

		GLenum internalFormat = GL_RGB;
		if (fourthColorFormat == GL_RGB16UI) { internalFormat = GL_RGB_INTEGER; }

		glTexImage2D(GL_TEXTURE_2D, 0, fourthColorFormat, 1, 1, 0, internalFormat, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, secondaryColor, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, thirdColor, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, fourthColor, 0);

		unsigned int attachments[4] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1,
			GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3};
		glDrawBuffers(4, attachments);

	}

	if (addDepth)
	{
		glGenTextures(1, &depth);
		glBindTexture(GL_TEXTURE_2D, depth);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, 1, 1,
			0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);

		if (!addColor)
		{
			glDrawBuffer(GL_NONE);
			glReadBuffer(GL_NONE);
		}

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth, 0);
	}

	// Check for framebuffer completeness
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "Framebuffer Error!!!\n";
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	if (addSecondaryRenderTarget)
	{
		glGenFramebuffers(1, &fboOnlyFirstTarget);
		glBindFramebuffer(GL_FRAMEBUFFER, fboOnlyFirstTarget);

		if (addColor)
		{
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color, 0);
		}

		if (addDepth)
		{
			if (!addColor)
			{
				glDrawBuffer(GL_NONE);
				glReadBuffer(GL_NONE);
			}

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth, 0);
		}

		// Check for framebuffer completeness
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			std::cout << "Framebuffer Error!!!\n";
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	if (addSecondaryRenderTarget)
	{
		glGenFramebuffers(1, &fboOnlySecondTarget);
		glBindFramebuffer(GL_FRAMEBUFFER, fboOnlySecondTarget);

		if (addColor)
		{
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, secondaryColor, 0);
		}

		if (addDepth)
		{
			if (!addColor)
			{
				glDrawBuffer(GL_NONE);
				glReadBuffer(GL_NONE);
			}

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth, 0);
		}

		// Check for framebuffer completeness
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			std::cout << "Framebuffer Error!!!\n";
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	if (addThirdRenderTarget)
	{
		glGenFramebuffers(1, &fboOnlyThirdTarget);
		glBindFramebuffer(GL_FRAMEBUFFER, fboOnlyThirdTarget);

		if (addColor)
		{
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, \
				thirdColor, 0);
		}

		if (addDepth)
		{
			if (!addColor)
			{
				glDrawBuffer(GL_NONE);
				glReadBuffer(GL_NONE);
			}

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth, 0);
		}

		// Check for framebuffer completeness
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			std::cout << "Framebuffer Error!!!\n";
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	if (addFourthRenderTarget)
	{
		glGenFramebuffers(1, &fboOnlyFourthTarget);
		glBindFramebuffer(GL_FRAMEBUFFER, fboOnlyFourthTarget);

		if (addColor)
		{
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, \
				fourthColor, 0);
		}

		if (addDepth)
		{
			if (!addColor)
			{
				glDrawBuffer(GL_NONE);
				glReadBuffer(GL_NONE);
			}

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth, 0);
		}

		// Check for framebuffer completeness
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			std::cout << "Framebuffer Error!!!\n";
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}



}

void Renderer::FBO::updateSize(int x, int y)
{
	if (size.x != x || size.y != y)
	{
		if (color)
		{
			glBindTexture(GL_TEXTURE_2D, color);
			glTexImage2D(GL_TEXTURE_2D, 0, colorFormat, x, y,
				0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		}

		if (secondaryColor)
		{
			glBindTexture(GL_TEXTURE_2D, secondaryColor);
			glTexImage2D(GL_TEXTURE_2D, 0, secondaryColorFormat, x, y,
				0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		}

		if (thirdColor)
		{
			GLenum internalFormat = GL_RGB;
			if (thirdColorFormat == GL_RGB16UI) { internalFormat = GL_RGB_INTEGER; }

			glBindTexture(GL_TEXTURE_2D, thirdColor);
			glTexImage2D(GL_TEXTURE_2D, 0, thirdColorFormat, x, y,
				0, internalFormat, GL_UNSIGNED_BYTE, NULL);
		}
		
		if (fourthColor)
		{
			GLenum internalFormat = GL_RGB;
			if (fourthColorFormat == GL_RGB16UI) { internalFormat = GL_RGB_INTEGER; }

			glBindTexture(GL_TEXTURE_2D, fourthColor);
			glTexImage2D(GL_TEXTURE_2D, 0, fourthColorFormat, x, y,
				0, internalFormat, GL_UNSIGNED_BYTE, NULL);
		}

		if (depth)
		{
			glBindTexture(GL_TEXTURE_2D, depth);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, x, y,
				0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		}

		size = glm::ivec2(x, y);
	}
}

void Renderer::FBO::copyDepthFromMainFBO(int w, int h)
{
	copyDepthFromOtherFBO(0, w, h);
}

void Renderer::FBO::copyColorFromMainFBO(int w, int h)
{
	copyColorFromOtherFBO(0, w, h);
}

void Renderer::FBO::copyDepthToMainFbo(int w, int h)
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_DEPTH_BUFFER_BIT);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);

	glBlitFramebuffer(
		0, 0, w, h,  // Source rectangle (usually the whole screen)
		0, 0, w, h,  // Destination rectangle (the size of your texture)
		GL_DEPTH_BUFFER_BIT, GL_NEAREST// You can adjust the filter mode as needed
	);
}

void Renderer::FBO::copyDepthFromOtherFBO(GLuint other, int w, int h)
{
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glClear(GL_DEPTH_BUFFER_BIT);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, other);
	//glBindTexture(GL_TEXTURE_2D, depth);

	glBlitFramebuffer(
		0, 0, w, h,  // Source rectangle (usually the whole screen)
		0, 0, w, h,  // Destination rectangle (the size of your texture)
		GL_DEPTH_BUFFER_BIT, GL_NEAREST// You can adjust the filter mode as needed
	);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::FBO::copyColorFromOtherFBO(GLuint other, int w, int h)
{
	glBindFramebuffer(GL_READ_FRAMEBUFFER, other);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);

	glBlitFramebuffer(
		0, 0, w, h,  // Source rectangle (usually the whole screen)
		0, 0, w, h,  // Destination rectangle (the size of your texture)
		GL_COLOR_BUFFER_BIT, GL_NEAREST// You can adjust the filter mode as needed
	);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


void Renderer::FBO::copyDepthAndColorFromOtherFBO(GLuint other, int w, int h)
{
	glBindFramebuffer(GL_READ_FRAMEBUFFER, other);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);

	glBlitFramebuffer(
		0, 0, w, h,  // Source rectangle (usually the whole screen)
		0, 0, w, h,  // Destination rectangle (the size of your texture)
		GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST// You can adjust the filter mode as needed
	);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::FBO::writeAllToOtherFbo(GLuint other, int w, int h)
{
	glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, other);

	glBlitFramebuffer(
		0, 0, w, h,  // Source rectangle (usually the whole screen)
		0, 0, w, h,  // Destination rectangle (the size of your texture)
		GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST// You can adjust the filter mode as needed
	);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::FBO::clearFBO()
{
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);


	if (depth && color)
	{
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	}
	else if (depth)
	{
		glClear(GL_DEPTH_BUFFER_BIT);
	}
	else if (color)
	{
		glClear(GL_COLOR_BUFFER_BIT);
	}

	if (secondaryColor)
	{
		const float clearColor2[] = {0.0f, 0.0f, 0.0f, 0.0f};
		glClearBufferfv(GL_COLOR, 1, clearColor2);
	}

	if (thirdColor)
	{
		const float clearColor3[] = {0.0f, 0.0f, 0.0f, 0.0f};
		glClearBufferfv(GL_COLOR, 2, clearColor3);
	}

	if (fourthColor)
	{
		const float clearColor3[] = {0.0f, 0.0f, 0.0f, 0.0f};
		glClearBufferfv(GL_COLOR, 3, clearColor3);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void QueryObject::create()
{
	glGenQueries(1, &id);
}

void QueryObject::begin()
{
	glBeginQuery(GL_SAMPLES_PASSED, id);
}

void QueryObject::end()
{
	glEndQuery(GL_SAMPLES_PASSED);
}

bool QueryObject::hasResult()
{
	GLint available = 0;
	glGetQueryObjectiv(id, GL_QUERY_RESULT_AVAILABLE, &available);
	return available == GL_TRUE;
}

int QueryObject::getQueryResult()
{
	if (!hasResult()) { return 0; }

	GLint result = 0;
	glGetQueryObjectiv(id, GL_QUERY_RESULT, &result);
	return result;
}

void QueryObject::clear()
{
	glDeleteQueries(1, &id);
	*this = {};
}

void AdaptiveExposure::update(float deltaTime, float newLuminosity)
{

	auto moveTowards = [&](float &current, float to, float speed)
	{
		float diff = to - current;

		if (diff < 0)
		{
			current -= deltaTime * speed;

			if (current < to)
			{
				current = to;
			}
		}
		else if (diff > 0)
		{
			current += deltaTime * speed;

			if (current > to)
			{
				current = to;
			}
		}
	};

	moveTowards(currentLuminosity, newLuminosity, 0.4);

	float newValue = linearRemap(currentLuminosity, 0, 1, maxExposure, minExposure);
	moveTowards(currentExposure, newValue, 0.1);


	float newValuebonusAmbient = linearRemap(currentLuminosity, 0, 0.3, 0.35, 0);
	moveTowards(bonusAmbient, newValuebonusAmbient, 0.1);

	//if (currentLuminosity > 0.50)
	//{
	//	currentExposure -= deltaTime * 0.2;
	//}
	//
	//if (currentLuminosity < 0.4)
	//{
	//	currentExposure += deltaTime * 0.2;
	//}

	currentExposure = glm::clamp(currentExposure, minExposure, maxExposure);
	bonusAmbient = glm::clamp(bonusAmbient, 0.f, 0.35f);



}
