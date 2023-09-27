#include "rendering/Ui.h"
#include <platform/platformInput.h>

namespace Ui
{
	static int xPadd = 0;
	static int yPadd = 0;
	static int width = 0;
	static int height = 0;

	Frame::Frame(glm::ivec4 size)
	{
		this->loaded = 1;
		lastW = width;
		lastH = height;
		lastX = xPadd;
		lastY = yPadd;

		width = size.z;
		height = size.w;
		xPadd = size.x;
		yPadd = size.y;
	}

	Frame::~Frame()
	{
		if(loaded)
		{
			width = lastW;
			height = lastH;
			xPadd = lastX;
			yPadd = lastY;
		}
	}

	glm::ivec4 Box::operator()()
	{

		if(dimensionsState == 1)
		{
			dimensions.w = dimensions.z * aspect;
		}else
		if(dimensionsState == 2)
		{
			dimensions.z = dimensions.w * aspect;
		}

		if(XcenterState == -1)
		{
			dimensions.x += xPadd;
		}
		if (YcenterState == -1)
		{
			dimensions.y += yPadd;
		}

		if(XcenterState == 1)
		{
			dimensions.x += xPadd + (width / 2) - (dimensions.z / 2);
		}
		if (YcenterState == 1)
		{
			dimensions.y += yPadd + (height/ 2) - (dimensions.w / 2);
		}
		
		if(XcenterState == 2)
		{
			dimensions.x += xPadd + width - dimensions.z;
		}

		if (YcenterState == 2)
		{
			dimensions.y += yPadd + height - dimensions.w;
		}


		return dimensions;
	}

	Box & Box::xDistancePixels(int dist)
	{
		dimensions.x = dist;
		XcenterState = 0;
		return *this;
	}

	Box & Box::yDistancePixels(int dist)
	{
		dimensions.y = dist;
		YcenterState = 0;
		return *this;
	}

	Box & Box::xCenter(int dist)
	{
		dimensions.x = dist;
		XcenterState = 1;
		return *this;
	}
	Box & Box::yCenter(int dist)
	{
		dimensions.y = dist;
		YcenterState = 1;
		return *this;
	}
	Box & Box::xLeft(int dist)
	{
		dimensions.x = dist;
		XcenterState = -1;
		return *this;
	}
	Box & Box::yTop(int dist)
	{
		dimensions.y = dist;
		YcenterState = -1;
		return *this;
	}
	Box & Box::xRight(int dist)
	{
		dimensions.x = dist;
		XcenterState = 2;
		return *this;
	}
	Box & Box::yBottom(int dist)
	{
		dimensions.y = dist;
		YcenterState = 2;
		return *this;
	}
	Box & Box::xDimensionPixels(int dim)
	{
		dimensionsState = 0;
		dimensions.z = dim;
		return *this;
	}
	Box & Box::yDimensionPixels(int dim)
	{
		dimensionsState = 0;
		dimensions.w = dim;
		return *this;
	}
	Box & Box::xDimensionPercentage(float p)
	{
		dimensionsState = 0;
		dimensions.z = p * width;
		return *this;

	}
	Box & Box::yDimensionPercentage(float p)
	{
		dimensionsState = 0;
		dimensions.w = p * height;
		return *this;
	}
	Box & Box::xAspectRatio(float r)
	{
		dimensionsState = 2;
		aspect = r;
		return *this;
	}
	Box & Box::yAspectRatio(float r)
	{
		dimensionsState = 1;
		aspect = r;
		return *this;
	}


	bool isInButton(const glm::vec2 & p, const glm::vec4 & box)
	{
		return(p.x >= box.x && p.x <= box.x + box.z
			&&
			p.y >= box.y && p.y <= box.y + box.w
			);
	}

	bool isButtonReleased(const glm::vec2 & p, const glm::vec4 & box)
	{
		return(p.x >= box.x && p.x <= box.x + box.z
			&&
			p.y >= box.y && p.y <= box.y + box.w
			) && platform::isLMouseReleased();
	}

}
