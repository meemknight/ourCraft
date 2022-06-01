#pragma once
#include "glm/vec4.hpp"
#include "glm/vec2.hpp"

namespace Ui
{
	class Frame
	{
		int lastW;
		int lastH;
		int lastX;
		int lastY;

		bool loaded = 0;
	
	public:
	
		Frame(const Frame &other) = delete;
		Frame(const Frame &&other) = delete;
		Frame(glm::ivec4 size);
		~Frame();
	};

	struct Box
	{
		glm::ivec4 dimensions = {};

		float aspect = 0;

		//-1 left
		// 0 none
		// 1 center
		// 2 right
		char XcenterState = 0;
		char YcenterState = 0;
		
		// 0 pixelSize
		// 1 xDominant
		// 2 yDominant
		char dimensionsState;

		//todo left percent
		Box &xDistancePixels(int dist);
		Box &yDistancePixels(int dist);
		Box &xCenter(int dist = 0);
		Box &yCenter(int dist = 0);
		Box &xLeft(int dist = 0);
		Box &yTop(int dist = 0);
		Box &xRight(int dist = 0);
		Box &yBottom(int dist = 0);

		Box &xDimensionPixels(int dim);
		Box &yDimensionPixels(int dim);

		Box &xDimensionPercentage(float p);
		Box &yDimensionPercentage(float p);

		Box &xAspectRatio(float r);
		Box &yAspectRatio(float r);

		glm::ivec4 operator()();

		operator glm::vec4 () { return (*this)(); }
	};

	bool isInButton(const glm::vec2 &p, const glm::vec4 &box);

	bool isButtonReleased(const glm::vec2 &p, const glm::vec4 &box);
};

