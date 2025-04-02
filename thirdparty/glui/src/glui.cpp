//////////////////////////////////////////////////
//gl2d.h				1.0.3
//Copyright(c) 2023 Luta Vlad
//https://github.com/meemknight/glui
//////////////////////////////////////////////////

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/color_space.hpp>

#include "glui/glui.h"
#include "gl2d/gl2d.h"
#include <unordered_map>
#include <iostream>
#include <sstream>


namespace glui
{

	static constexpr bool MINECRAFT_LOOK_SLIDER = 1;


	void defaultErrorFunc(const char *msg)
	{
		std::cerr << "glui error: " << msg << "\n";
	}

	static errorFuncType *errorFunc = defaultErrorFunc;

	errorFuncType *setErrorFuncCallback(errorFuncType *newFunc)
	{
		auto a = errorFunc;
		errorFunc = newFunc;
		return a;
	}

	enum widgetType
	{
		none = 0,
		button,
		toggle,
		toggleButton,
		text,
		textInput,
		beginMenu,
		beginManualMenu,
		startManualMenu,
		exitCurrentMenu,
		endMenu,
		texture,
		buttonWithTexture,
		sliderFloatW,
		colorPickerW,
		newColumW,
		sliderIntW,
		sliderUint8,
		sliderint8,
		customWidget,
		optionsToggle,
	};





	bool aabb(glm::vec4 transform, glm::vec2 point)
	{
		if (
			point.x >= transform.x &&
			point.y >= transform.y &&
			point.x <= transform.x + transform.z &&
			point.y <= transform.y + transform.w
			)
		{
			return true;
		}
		else
		{
			return false;
		}
	}



	constexpr float pressDownSize = 0.04f;
	constexpr float shadowSize = 0.1f;
	constexpr float outlineSize = 0.02f;
	constexpr float textFit = 64.f;

	constexpr float nonMinimizeTextSize = 0.9f;
	constexpr float minimizeRatio = 0.8f;

	constexpr float buttonFit = 0.6f;

	constexpr float inSizeY = 0.8;
	constexpr float inSizeX = 0.8;
	constexpr float mainInSizeX = 0.9;
	constexpr float mainInSizeY = 0.9;
	constexpr float paddingColums = 0.9;

	void splitTransforms(glm::vec4 &down, glm::vec4 &newTransform, glm::vec4 transform)
	{
		down = transform;
		newTransform = transform;
		float border = shadowSize * std::min(transform.w, transform.z);
		down.w = border;
		newTransform.w -= border;
		down.y += newTransform.w;
	}

	glm::vec4 stepColorUp(glm::vec4 color, float perc)
	{
		glm::vec3 hsv = glm::hsvColor(glm::vec3(color));

		if (hsv.b >= 1.f)
		{
			hsv.g = std::max(0.f, hsv.g - perc * 2);
		}
		else
		{
			hsv.b = std::min(1.f, hsv.b + perc);
		}

		hsv = glm::rgbColor(hsv);
		color.r = hsv.r;
		color.g = hsv.g;
		color.b = hsv.b;

		return color;
	}
	glm::vec4 stepColorDown(glm::vec4 color, float perc)
	{
		color.r *= perc;
		color.g *= perc;
		color.b *= perc;
		return color;
	}

	std::string getString(std::string s)
	{
		auto f = s.find("##");
		if (f != s.npos)
		{
			s = std::string(s.begin(), s.begin() + f);
		}
		return s;
	}

	void renderFancyBox(gl2d::Renderer2D &renderer,
		glm::vec4 transform, glm::vec4 color, gl2d::Texture t, bool hovered, bool clicked)
	{
		if (color.a <= 0.01f) { return; }

		float colorDim = 0.f;
		if (hovered)
		{
			colorDim = 0.2f;
			if (clicked)
			{
				colorDim = -0.8f;
			}
		}

		glm::vec4 newColor = {};
		if (colorDim > 0)
		{
			newColor = stepColorUp(color, colorDim);
		}
		else if (colorDim < 0)
		{
			newColor = stepColorDown(color, -colorDim); //todo refactor this functions
		}
		else
		{
			newColor = color;
		}

		auto lightColor = stepColorUp(newColor, 0.02);
		auto darkColor = stepColorDown(newColor, 0.5f);
		auto darkerColor = stepColorDown(newColor, 0.25f);

		auto outlineColor = stepColorUp(newColor, 0.3);
		//auto outlineColor = darkerColor;

		glm::vec4 colorVector[4] = {darkColor, darkColor, lightColor, lightColor};

		if (t.id == 0)
		{

			float calculatedOutline = outlineSize * std::min(transform.w, transform.z);
			if (hovered)
			{
				renderer.renderRectangle(transform, outlineColor);
			}
			else
			{
				renderer.renderRectangle(transform, darkColor);
			}
			transform.x += calculatedOutline;
			transform.y += calculatedOutline;
			transform.z -= calculatedOutline * 2;
			transform.w -= calculatedOutline * 2;

			glm::vec4 middle = {};
			glm::vec4 down = {};
			splitTransforms(down, middle, transform);
			renderer.renderRectangle(middle, colorVector);
			renderer.renderRectangle(down, darkerColor);

		}
		else
		{
			//renderer.renderRectangle(transform, newColor, {}, 0.f, t);
			//renderer.render9Patch2(transform, newColor, {}, 0.f, t, GL2D_DefaultTextureCoords, {2.f/26.f, 24.f / 26.f,24.f / 26.f,2.f / 26.f});
			//renderer.render9Patch2(transform, newColor, {}, 0.f, t, GL2D_DefaultTextureCoords, {0.2,0.8,0.8,0.2});
			renderer.render9Patch(transform, 20, newColor, {}, 0.f, t, GL2D_DefaultTextureCoords, {0.2,0.8,0.8,0.2});

		}
	}

	//just z and w components of transform used
	//todo: move into gl2d a function to render text of a size
	float determineTextSize(gl2d::Renderer2D &renderer, const std::string &str,
		gl2d::Font &f, glm::vec4 transform, bool minimize = true)
	{
		auto newStr = getString(str);
		float size = textFit;

		auto s = renderer.getTextSize(newStr.c_str(), f, size);

		float ratioX = transform.z / s.x;
		float ratioY = transform.w / s.y;


		if (ratioX > 1 && ratioY > 1)
		{

			///keep size
			//return size;

			//else
			//{
			//	if (ratioX > ratioY)
			//	{
			//		return size * ratioY;
			//	}
			//	else
			//	{
			//		return size * ratioX;
			//	}
			//}

		}
		else
		{
			if (ratioX < ratioY)
			{
				size *= ratioX;
			}
			else
			{
				size *= ratioY;
			}
		}

		if (minimize)
		{
			size *= minimizeRatio;
		}
		else
		{
			size *= nonMinimizeTextSize;
		}

		return size;
	}

	glm::vec4 determineTextPos(gl2d::Renderer2D &renderer, const std::string &str, gl2d::Font &f, glm::vec4 transform,
		bool noTexture, bool minimize = true)
	{
		auto newStr = getString(str);
		auto newS = determineTextSize(renderer, newStr, f, transform);

		auto s = renderer.getTextSize(newStr.c_str(), f, newS);

		glm::vec2 pos = glm::vec2(transform);

		pos.x += transform.z / 2.f;
		pos.y += transform.w / 2.f;

		pos -= s / 2.f;

		return glm::vec4{pos, s};
	}

	//todo reuse the upper function
	void renderText(gl2d::Renderer2D &renderer, const std::string &str,
		gl2d::Font &f, glm::vec4 transform, glm::vec4 color,
		bool noTexture, bool minimize, bool alignLeft, float maxSize)
	{
		auto newStr = getString(str);
		auto newS = determineTextSize(renderer, newStr, f, transform, minimize);

		if (newS > maxSize && maxSize != 0)
		{
			newS = maxSize;
		}

		glm::vec2 pos = glm::vec2(transform);

		if (!alignLeft)
		{
			pos.x += transform.z / 2.f;
			pos.y += transform.w / 2.f;
			renderer.renderText(pos, newStr.c_str(), f, color, newS, 4, 3);
		}
		else
		{
			//pos.x += transform.z * 0.02;
			//pos.y += transform.w * 0.4;
			pos.y += transform.w / 2.f;
			renderer.renderText(pos, newStr.c_str(), f, color, newS, 4, 3, false);
		}

	}


	void renderTextInput(gl2d::Renderer2D &renderer, const std::string &str,
		char *text, size_t textSizeWithNullChar, const std::string &typedInput,
		gl2d::Font &f, glm::vec4 transform, glm::vec4 colors, const gl2d::Texture texture,
		bool displayText,
		bool enabled
	)
	{
		size_t n = textSizeWithNullChar;
		int pos = strlen(text);

		bool hovered = 0;
		bool clicked = 0;

		if (enabled)
		{
			for (auto i : typedInput)
			{
				if (i == 8) //backspace
				{
					if (pos > 0)
					{
						pos--;
						text[pos] = 0;
					}
				}
				else if (i == '\n')
				{
					//ignore
				}
				else
				{
					if (pos < n - 1)
					{
						text[pos] = i;
						pos++;
						text[pos] = 0;
					}
				}
			}
		}

		if (texture.id != 0)
		{
			renderFancyBox(renderer, transform,
				colors, texture, hovered, clicked);
		}

		std::string textCopy = text;

		if (displayText)
		{
			textCopy = getString(str) + textCopy;
		}

		if (enabled)
		{
			//if ((int)timer % 2)
			//{
			//	textCopy += "|";
			//}
		}

		renderText(renderer, textCopy, f, transform, Colors_White, true,
			!hovered);

	}


	glm::vec4 computeTextureNewPosition(glm::vec4 transform, gl2d::Texture t)
	{
		auto tsize = t.GetSize();

		if (tsize.y == 0) { return {}; }
		if (transform.w == 0) { return {}; }

		float aspectRatio = tsize.x / (float)tsize.y;
		float boxAspectRatio = transform.z / transform.w;

		if (aspectRatio < boxAspectRatio) // the texture is shorter than the box 
		{
			glm::vec2 newSize = {};
			newSize.y = transform.w;
			newSize.x = aspectRatio * newSize.y;

			glm::vec4 newPos = {transform.x, transform.y, newSize};
			newPos.x += (transform.z - newSize.x) / 2.f;

			return newPos;
		}
		else if (aspectRatio > boxAspectRatio) //the texture is longer than the box
		{
			glm::vec2 newSize = {};
			newSize.x = transform.z;
			newSize.y = newSize.x / aspectRatio;

			glm::vec4 newPos = {transform.x, transform.y, newSize};
			newPos.y += (transform.w - newSize.y) / 2.f;

			return newPos;
		}
		else // if (aspectRatio == boxAspectRatio) // redundant
		{
			return transform;
		}
	}

	void renderTexture(gl2d::Renderer2D &renderer, glm::vec4 transform, gl2d::Texture t, gl2d::Color4f c,
		glm::vec4 textureCoordonates)
	{
		auto newPos = computeTextureNewPosition(transform, t);

		renderer.renderRectangle(newPos, t, c);
	}

	bool renderSliderFloat(gl2d::Renderer2D &renderer, glm::vec4 transform, float *value, float min, float max,
		bool &sliderBeingDragged,
		gl2d::Texture barT, gl2d::Color4f barC, gl2d::Texture ballT,
		gl2d::Color4f ballC, RendererUi::Internal::InputData &input)
	{

		bool returnVal = 0;
		float barSize = 7;
		float barIndent = 16;
		float bulletSize = 14.f;

		if (MINECRAFT_LOOK_SLIDER)
		{
			barSize = transform.w;
			barIndent = 0;
			bulletSize = transform.w;
		}

		glm::vec4 barTransform(transform.x + barIndent, transform.y + (transform.w - barSize) / 2.f,
			transform.z - barIndent * 2.f, barSize);

		glm::vec4 bulletTransform(barTransform.x, barTransform.y + (barSize - bulletSize) / 2.f,
			bulletSize / 2.f, bulletSize);

		bulletTransform.x += std::max(std::min((*value - min) / (max - min), 1.f), 0.f)
			* (barTransform.z - bulletTransform.z);

		//todo color
		renderFancyBox(renderer, barTransform, barC, barT, 0, 0);

		bool hovered = false;
		bool clicked = false;

		if (sliderBeingDragged == true && input.mouseHeld)
		{
			hovered = true;
			clicked = true;
		}
		else
		{
			if (aabb(barTransform, input.mousePos))
			{
				hovered = true;

				if (input.mouseClick)
				{
					clicked = true;
				}
			}
		}

		if (clicked)
		{
			sliderBeingDragged = true;

			float ballSizeHalf = bulletTransform.z / 2;

			int begin = barTransform.x + ballSizeHalf;
			int end = barTransform.x + barTransform.z - ballSizeHalf;

			int mouseX = input.mousePos.x;


			float mouseVal = (mouseX - (float)begin) / (end - (float)begin);

			mouseVal = glm::clamp(mouseVal, 0.f, 1.f);

			mouseVal *= max - min;
			mouseVal += min;

			if (*value != mouseVal)
			{
				returnVal = true;
			}
			*value = mouseVal;
		}
		else
		{
			sliderBeingDragged = false;
		}

		renderFancyBox(renderer, bulletTransform, ballC, ballT,
			hovered, clicked);

		return returnVal;
	}

	bool renderSliderInt(gl2d::Renderer2D &renderer, glm::vec4 transform, int *value, int min, int max,
		bool &sliderBeingDragged,
		gl2d::Texture barT, gl2d::Color4f barC, gl2d::Texture ballT, gl2d::Color4f ballC, RendererUi::Internal::InputData &input)
	{

		bool returnVal = 0;
		float barSize = 7;
		float barIndent = 16;
		float bulletSize = 14.f;

		if (MINECRAFT_LOOK_SLIDER)
		{
			barSize = transform.w;
			barIndent = 0;
			bulletSize = transform.w;
		}

		glm::vec4 barTransform(transform.x + barIndent, transform.y + (transform.w - barSize) / 2.f,
			transform.z - barIndent * 2.f, barSize);

		glm::vec4 bulletTransform(barTransform.x, barTransform.y + (barSize - bulletSize) / 2.f,
			bulletSize / 2.f, bulletSize);


		bulletTransform.x += std::max(std::min((*value - min) / (float)(max - min), 1.f), 0.f)
			* (barTransform.z - bulletTransform.z);

		//todo color
		renderFancyBox(renderer, barTransform, barC, barT, 0, 0);

		bool hovered = false;
		bool clicked = false;

		if (sliderBeingDragged == true && input.mouseHeld)
		{
			hovered = true;
			clicked = true;
		}
		else
		{
			if (aabb(barTransform, input.mousePos))
			{
				hovered = true;

				if (input.mouseClick)
				{
					clicked = true;
				}
			}
		}

		if (clicked)
		{
			sliderBeingDragged = true;

			float ballSizeHalf = bulletTransform.z / 2;
			int begin = barTransform.x + ballSizeHalf;
			int end = barTransform.x + barTransform.z - ballSizeHalf;


			int mouseX = input.mousePos.x;

			float mouseVal = (mouseX - (float)begin) / (end - (float)begin);

			mouseVal = glm::clamp(mouseVal, 0.f, 1.f);

			mouseVal *= max - min;
			mouseVal += min;

			if (*value != mouseVal)
			{
				returnVal = true;
			}
			*value = mouseVal;
		}
		else
		{
			sliderBeingDragged = false;
		}

		renderFancyBox(renderer, bulletTransform, ballC, ballT,
			hovered, clicked);

		return returnVal;
	}

	template<class T>
	bool renderSliderGeneric(gl2d::Renderer2D &renderer, glm::vec4 transform, T *value, T min, T max,
		bool &sliderBeingDragged,
		gl2d::Texture barT, gl2d::Color4f barC, gl2d::Texture ballT, gl2d::Color4f ballC, RendererUi::Internal::InputData &input)
	{

		bool returnVal = 0;
		float barSize = 7;
		float barIndent = 16;
		float bulletSize = 14.f;

		if (MINECRAFT_LOOK_SLIDER)
		{
			barSize = transform.w;
			barIndent = 0;
			bulletSize = transform.w;
		}

		glm::vec4 barTransform(transform.x + barIndent, transform.y + (transform.w - barSize) / 2.f,
			transform.z - barIndent * 2.f, barSize);

		glm::vec4 bulletTransform(barTransform.x, barTransform.y + (barSize - bulletSize) / 2.f,
			bulletSize / 2.f, bulletSize);


		bulletTransform.x += std::max(std::min((*value - min) / (float)(max - min), 1.f), 0.f)
			* (barTransform.z - bulletTransform.z);

		//todo color
		renderFancyBox(renderer, barTransform, barC, barT, 0, 0);

		bool hovered = false;
		bool clicked = false;

		if (sliderBeingDragged == true && input.mouseHeld)
		{
			hovered = true;
			clicked = true;
		}
		else
		{
			if (aabb(barTransform, input.mousePos))
			{
				hovered = true;

				if (input.mouseClick)
				{
					clicked = true;
				}
			}
		}

		if (clicked)
		{
			sliderBeingDragged = true;

			float ballSizeHalf = bulletTransform.z / 2;
			int begin = barTransform.x + ballSizeHalf;
			int end = barTransform.x + barTransform.z - ballSizeHalf;


			int mouseX = input.mousePos.x;

			float mouseVal = (mouseX - (float)begin) / (end - (float)begin);

			mouseVal = glm::clamp(mouseVal, 0.f, 1.f);

			mouseVal *= max - min;
			mouseVal += min;

			if (*value != (T)mouseVal)
			{
				returnVal = true;
			}
			*value = (T)mouseVal;
		}
		else
		{
			sliderBeingDragged = false;
		}

		renderFancyBox(renderer, bulletTransform, ballC, ballT,
			hovered, clicked);

		return returnVal;
	}


	bool toggleOptions(gl2d::Renderer2D &renderer, glm::vec4 transform, const std::string &text, glm::vec4 textColor,
		const std::string &optionsSeparatedByBars, int *currentIndex, bool showText,
		gl2d::Font &font, gl2d::Texture &texture, gl2d::Color4f textureColor,
		glm::ivec2 mousePos, bool mouseHeld, bool mouseReleased,
		gl2d::Color4f *optionsColors, std::string toolTip)
	{

		auto transformDrawn = transform;
		auto aabbTransform = transform;
		bool hovered = 0;
		bool clicked = 0;
		bool retValue = 0;

		int *index = (int *)currentIndex;

		int stub = 0;
		if (!index) { index = &stub; errorFunc("Error, nullptr passed as an index for toggleOptions!"); }

		if (textColor.a <= 0.01f)
		{
			auto p = determineTextPos(renderer, text, font, transformDrawn, true);
			aabbTransform = p;
		}

		int maxSize = 1;
		for (int i = 0; i < optionsSeparatedByBars.size(); i++)
		{
			char c = optionsSeparatedByBars[i];
			if (c == '|')
			{
				maxSize++;
			}
		}

		if (optionsSeparatedByBars.empty()) { maxSize = 0; }

		if (*index > maxSize - 1)
		{
			*index = 0;
		}

		glm::vec4 newTextColor = textColor;

		if (optionsColors)
		{
			newTextColor = ((glm::vec4 *)optionsColors)[*index];
		}

		if (aabb(aabbTransform, mousePos))
		{
			hovered = true;
			if (mouseHeld)
			{
				clicked = true;
				transformDrawn.y += transformDrawn.w * pressDownSize;
			}
		}

		if (hovered && newTextColor.a <= 0.01f)
		{
			newTextColor = stepColorDown(newTextColor, 0.8);
		}

		if (mouseReleased && aabb(aabbTransform, mousePos))
		{
			retValue = true;
			(*index)++;
		}

		if (*index > maxSize - 1)
		{
			*index = 0;
		}

		renderFancyBox(renderer, transformDrawn,
			textureColor, texture, hovered, clicked);

		std::string finalText;

		if (showText)
		{
			finalText = text;
		}

		int currentIncrement = 0;
		for (int i = 0; i < optionsSeparatedByBars.size(); i++)
		{
			if (currentIncrement == *index)
			{

				char c = optionsSeparatedByBars[i];
				if (c == '|')
				{
					break;
				}
				finalText += c;
			}

			char c = optionsSeparatedByBars[i];
			if (c == '|')
			{
				currentIncrement++;
			}
		}

		if ((newTextColor.a <= 0.01f || texture.id == 0))
		{
			renderText(renderer, finalText, font,
				transformDrawn, newTextColor, true, !hovered);
		}
		else
		{
			renderText(renderer, finalText,
				font, transformDrawn, newTextColor, false, !hovered);
		}

		if (!toolTip.empty() && hovered)
		{
			glm::vec4 transform = transformDrawn;
			transform.x += transform.z * 0.1;
			transform.y += transform.w * 1.1f;

			int lines = 1;
			for (auto &c : toolTip)
			{
				if (c == '\n' || c == '\v')
				{
					lines++;
				}
			}

			transform.w *= lines;

			renderFancyBox(renderer, transform,
				stepColorDown(textureColor, 0.8)
				, texture, 0, 0);

			transform.x += transform.z * 0.1f;
			transform.y += transform.w * 0.1f;
			transform.z *= 0.9f;
			transform.w *= 0.9f;

			transform.w /= lines;

			int ind = 0;
			std::string copy = "";
			for (int l = 1; l <= lines;)
			{
				if (toolTip[ind] == '\n' ||
					toolTip[ind] == '\v'
					)
				{
					renderText(renderer, copy,
						font, transform, textColor, 0, true, true);
					l++;
					copy = "";
					transform.y += transform.w;
				}
				else
				{
					copy += toolTip[ind];
				}

				ind++;

				if (ind >= toolTip.size())break;
			}

			renderText(renderer, copy,
				font, transform, textColor, 0, true, true);
		}

		return retValue;
	}


	float timer = 0;
	bool idWasSet = 0;


	bool drawButton(gl2d::Renderer2D &renderer, glm::vec4 transform, glm::vec4 color,
		const std::string &s,
		gl2d::Font &font, gl2d::Texture &texture, glm::ivec2 mousePos, bool mouseHeld, bool mouseReleased)
	{
		auto transformDrawn = transform;
		auto aabbTransform = transform;
		bool hovered = 0;
		bool clicked = 0;
		auto textColor = Colors_White;

		if (color.a <= 0.01f)
		{
			auto p = determineTextPos(renderer, s, font, transformDrawn, true);
			aabbTransform = p;
		}

		if (aabb(aabbTransform, mousePos))
		{
			hovered = true;
			if (mouseHeld)
			{
				clicked = true;
				transformDrawn.y += transformDrawn.w * pressDownSize;
			}
		}

		if (hovered && color.a <= 0.01f)
		{
			textColor = stepColorDown(textColor, 0.8);
		}

		bool rez = 0;

		if (mouseReleased && aabb(aabbTransform, mousePos))
		{
			rez = true;
		}
		else
		{
			rez = false;
		}

		renderFancyBox(renderer, transformDrawn, color, texture, hovered, clicked);

		if ((color.a <= 0.01f || texture.id == 0))
		{
			renderText(renderer, s, font, transformDrawn, textColor, true, !hovered);
		}
		else
		{
			renderText(renderer, s, font, transformDrawn, textColor, false, !hovered);
		}

		return rez;
	};


	void RendererUi::renderFrame(gl2d::Renderer2D &renderer,
		gl2d::Font &font, glm::ivec2 mousePos, bool mouseClick,
		bool mouseHeld, bool mouseReleased,
		bool escapeReleased, const std::string &typedInput,
		float deltaTime,
		bool *anyButtonPressed, bool *backPressed, bool *anyCustomWidgetPressed
		, bool *anyToggleToggeled, bool *anyToggleDetoggeled, bool *andSliderDragged
	)
	{
		if (anyButtonPressed) { *anyButtonPressed = 0; }
		if (backPressed) { *backPressed = 0; }
		if (anyCustomWidgetPressed) { *anyCustomWidgetPressed = 0; }
		if (anyToggleToggeled) { *anyToggleToggeled = 0; }
		if (anyToggleDetoggeled) { *anyToggleDetoggeled = 0; }
		if (andSliderDragged) { *andSliderDragged = 0; }

		if (!idWasSet)
		{
			return;
		}
		//find the menu stack for this Begin()

		auto iterMenuStack = internal.allMenuStacks.find(internal.currentId);
		if (iterMenuStack == internal.allMenuStacks.end())
		{
			iterMenuStack = internal.allMenuStacks.insert({internal.currentId, {}}).first;
		}
		auto &currentMenuStack = iterMenuStack->second;

		idWasSet = 0;
		internal.currentId = 0;

		if (escapeReleased && !currentMenuStack.empty())
		{
			currentMenuStack.pop_back();

			if (backPressed)
			{
				*backPressed = true;
			}
		}

		timer += deltaTime * 2;
		if (timer >= 2.f)
		{
			timer -= 2;
		}

		//clear some data
		for (auto &i : internal.widgets)
		{
			if (i.second.type == widgetType::customWidget)
			{
				i.second.customWidgetUsed = false;
			}
		}

		std::vector<std::pair<std::string, Internal::Widget>> widgetsCopy;
		widgetsCopy.reserve(internal.widgetsVector.size());

		std::vector<std::string> traversedStack;
		traversedStack.reserve(currentMenuStack.size());

		for (auto &i : internal.widgetsVector)
		{
			bool add = 0;
			if (traversedStack.size() == currentMenuStack.size())
			{
				add = true;
				for (int i = 0; i < traversedStack.size(); i++)
				{
					if (traversedStack[i] != currentMenuStack[i])
					{
						add = false;
						break;
					}
				}
			}

			if (add && i.second.type != widgetType::beginManualMenu)
			{
				widgetsCopy.emplace_back(i);
			}

			if (i.second.type == widgetType::beginMenu
				|| i.second.type == widgetType::beginManualMenu)
			{
				traversedStack.push_back(i.first);
			}
			else if (i.second.type == widgetType::endMenu)
			{
				traversedStack.pop_back();
			}

		}

		/*
		auto currentMenuStackCopy = currentMenuStack;
		{
			std::vector<std::string> menuStack;

			std::string nextMenu = "";
			bool shouldIgnor = false;

			if (!currentMenuStackCopy.empty())
			{
				nextMenu = currentMenuStackCopy.front();
				currentMenuStackCopy.erase(currentMenuStackCopy.begin());
				shouldIgnor = true;
			}

			int nextStackSizeToLook = 0;
			int nextStackSizeToLookMin = 0;

			for (auto& i : internal.widgetsVector)
			{

				if (i.second.type == widgetType::beginMenu)
				{
					menuStack.push_back(i.first);

					if (i.first == nextMenu)
					{
						if (!currentMenuStackCopy.empty())
						{
							nextMenu = currentMenuStackCopy.front();
							currentMenuStackCopy.erase(currentMenuStackCopy.begin());
							shouldIgnor = true;

						}
						else
						{
							shouldIgnor = false;
							nextStackSizeToLookMin = menuStack.size();
						}
					}else
					if (i.first != nextMenu && shouldIgnor != true)
					{
						shouldIgnor = true;
						nextStackSizeToLook = menuStack.size() - 1;
						widgetsCopy.push_back(i);
					}

					continue;
				}

				if (i.second.type == widgetType::endMenu)
				{
					i.first = "##$" + menuStack.back();
					menuStack.pop_back();

					if (nextStackSizeToLook == menuStack.size())
					{
						shouldIgnor = false;
					}

					if (menuStack.size() < nextStackSizeToLookMin)
					{
						shouldIgnor = true;
					}

					continue;
				}

				if (shouldIgnor)
				{
					continue;
				}

				widgetsCopy.push_back(i);

			}
		};
		*/

		auto computePos = [&](int elementsHeight, float &advanceSizeY)
		{

			glm::vec4 viewFrame(0, 0, renderer.windowW, renderer.windowH);
			if (temporalViewPort) { viewFrame = *temporalViewPort; }

			float sizeWithPaddY = 0;

			if (internal.alignSettings.widgetSize.y != 0)
			{
				sizeWithPaddY = std::min(internal.alignSettings.widgetSize.y,
					((float)viewFrame.w / elementsHeight));
			}
			else
			{
				sizeWithPaddY = ((float)viewFrame.w / elementsHeight);
			}

			float sizeY = sizeWithPaddY * inSizeY;
			float paddSizeY = sizeWithPaddY * (1 - inSizeY) / 2.f;

			float sizeWithPaddX = (float)viewFrame.z;
			float sizeX = sizeWithPaddX * inSizeX;
			float paddSizeX = sizeWithPaddX * (1 - inSizeX) / 2.f;

			glm::vec4 computedPos = {};
			computedPos.x = paddSizeX + (float)viewFrame.z * (1 - mainInSizeX) * 0.5f + viewFrame.x;
			computedPos.y = paddSizeY + (float)viewFrame.w * (1 - mainInSizeY) * 0.5f + viewFrame.y;
			computedPos.z = sizeX * mainInSizeX;
			computedPos.w = sizeY * mainInSizeY;

			advanceSizeY = (paddSizeY * 2 + sizeY) * mainInSizeY;

			return computedPos;
		};

		std::vector<std::pair<glm::vec4, float>> colums;

		int widgetsCountUntillNewColumW = 0;

		for (int i = 0; i < widgetsCopy.size(); i++)
		{
			if (widgetsCopy[i].second.type == glui::widgetType::newColumW)
			{
				float padd = 0;
				auto rez = computePos(widgetsCountUntillNewColumW, padd);
				colums.push_back({rez, padd});
				widgetsCountUntillNewColumW = 0;
			}
			else
			{
				widgetsCountUntillNewColumW++;
			}
		}
		if (widgetsCountUntillNewColumW != 0)
		{
			float padd = 0;
			auto rez = computePos(widgetsCountUntillNewColumW, padd);
			colums.push_back({rez, padd});
			widgetsCountUntillNewColumW = 0;
		}

		int currentColum = 0;

		for (auto &i : colums)
		{
			i.first.z /= colums.size();
		}

		float columAdvanceSize = colums[0].first.z; //all colums have the same width for now
		float beginY = colums[0].first.y; //all colums start from the same height fot now

		for (int i = 0; i < colums.size(); i++)
		{
			colums[i].first.x += columAdvanceSize * i;

			colums[i].first.x += colums[i].first.z * ((1.f - paddingColums) / 2.f);
			colums[i].first.z *= paddingColums;
		}

		auto camera = renderer.currentCamera;
		renderer.currentCamera.setDefault();

		Internal::InputData input = {};
		input.mousePos = mousePos;
		input.mouseClick = mouseClick;
		input.mouseHeld = mouseHeld;
		input.mouseReleased = mouseReleased;
		input.escapeReleased = escapeReleased;


		for (auto &i : widgetsCopy)
		{

			auto find = internal.widgets.find(i.first);

			if (find == internal.widgets.end())
			{

				i.second.usedThisFrame = true;
				i.second.justCreated = true;
				internal.widgets.insert(i);

				//continue;
			}
			else
			{

				if (find->second.type != i.second.type)
				{
					errorFunc("reupdated a widget with a different type");
					//todo warning or nothing at all?
				}

				if (find->second.usedThisFrame == true)
				{
					errorFunc("used a widget name more than once, use name##unique_id");
				}

				auto pd = find->second.pd;

				find->second = i.second;
				find->second.justCreated = false;
				find->second.pd = pd;

				find->second.usedThisFrame = true;
				//continue;
			}

			{
				auto &j = *internal.widgets.find(i.first);
				auto &widget = j.second;

				auto drawButtonImpl = [&]()
				{
					auto rez = drawButton(renderer, colums[currentColum].first,
						widget.colors, j.first, font, widget.texture, input.mousePos,
						input.mouseHeld, input.mouseReleased);

					if (rez)
					{
						if (anyButtonPressed) { *anyButtonPressed = true; }
					}

					widget.returnFromUpdate = rez;

					return rez;
				};


				switch (widget.type)
				{
				case widgetType::button:
				{

					drawButtonImpl();

					break;
				}
				case widgetType::toggle:
				{
					auto transformDrawn = colums[currentColum].first;
					bool hovered = 0;
					bool clicked = 0;

					glm::vec4 toggleTransform = transformDrawn;
					glm::vec4 textTransform = transformDrawn;
					textTransform.z -= toggleTransform.w;
					toggleTransform.z = toggleTransform.w;

					auto p = determineTextPos(renderer, j.first, font, textTransform, true);
					toggleTransform.x = p.x + p.z;

					glm::vec4 aabbBox = p;
					aabbBox.z += toggleTransform.z;
					aabbBox.y = std::min(toggleTransform.y, textTransform.y);
					aabbBox.w = std::max(toggleTransform.w, textTransform.w);

					if (aabb(aabbBox, input.mousePos))
					{
						hovered = true;
						if (input.mouseHeld)
						{
							clicked = true;
							textTransform.y += transformDrawn.w * pressDownSize;
							toggleTransform.y += transformDrawn.w * pressDownSize;
						}
					}

					if (input.mouseReleased && aabb(aabbBox, input.mousePos))
					{
						*(bool *)(widget.pointer) = !(*(bool *)(widget.pointer));

						if (*(bool *)(widget.pointer))
						{
							if (anyToggleToggeled)
							{
								*anyToggleToggeled = true;
							}
						}
						else
						{
							if (anyToggleDetoggeled)
							{
								*anyToggleDetoggeled = true;
							}
						}
					}

					widget.returnFromUpdate = *(bool *)(widget.pointer);


					if (hovered)
					{
						renderText(renderer, j.first, font, textTransform, stepColorDown(Colors_White, 0.8),
							true, false);
					}
					else
					{
						renderText(renderer, j.first, font, textTransform, Colors_White, true);
					}


					if (widget.returnFromUpdate)
					{
						auto small = toggleTransform;
						small.z *= buttonFit;
						small.w *= buttonFit;
						small.x += toggleTransform.z * (1.f - buttonFit) / 2.f;
						small.y += toggleTransform.w * (1.f - buttonFit) / 2.f;

						renderFancyBox(renderer, toggleTransform, widget.colors, widget.texture, hovered, clicked);

						if (widget.textureOver.id)
						{
							renderFancyBox(renderer, toggleTransform, Colors_White, widget.textureOver, false, false);
						}
						else
						{
							renderFancyBox(renderer, small, widget.colors, widget.textureOver, false, false);
						}
					}
					else
					{
						renderFancyBox(renderer, toggleTransform, widget.colors, widget.texture, hovered, clicked);
					}



					break;
				}
				case widgetType::toggleButton:
				{
					auto transformDrawn = colums[currentColum].first;
					bool hovered = 0;
					bool clicked = 0;

					glm::vec4 toggleTransform = transformDrawn;
					glm::vec4 textTransform = transformDrawn;

					if (aabb(toggleTransform, input.mousePos))
					{
						hovered = true;
						if (input.mouseHeld)
						{
							clicked = true;
							textTransform.y += transformDrawn.w * pressDownSize;
							toggleTransform.y += transformDrawn.w * pressDownSize;
						}
					}

					if (input.mouseReleased && aabb(toggleTransform, input.mousePos))
					{
						*(bool *)(widget.pointer) = !(*(bool *)(widget.pointer));

						if (*(bool *)(widget.pointer))
						{
							if (anyToggleToggeled)
							{
								*anyToggleToggeled = true;
							}
						}
						else
						{
							if (anyToggleDetoggeled)
							{
								*anyToggleDetoggeled = true;
							}
						}
					}

					widget.returnFromUpdate = *(bool *)(widget.pointer);

					renderFancyBox(renderer,
						toggleTransform, widget.colors2,
						widget.texture, hovered, clicked);

					std::string text = getString(j.first);

					if (widget.returnFromUpdate)
					{
						text += ": ON";
					}
					else
					{
						text += ": OFF";
					}

					if (hovered)
					{
						renderText(renderer, text, font, textTransform,
							stepColorDown(Colors_White, 0.8),
							true, false);
					}
					else
					{
						renderText(renderer, text, font, textTransform,
							Colors_White, true);
					}


					break;
				}
				case widgetType::text:
				{

					renderText(renderer, j.first, font, colums[currentColum].first, j.second.colors, true);

					break;
				}
				case widgetType::textInput:
				{

					char *text = (char *)j.second.pointer;
					size_t n = j.second.textSize;

					int pos = strlen(text);

					bool enabled = j.second.enabeled;

					auto transform = colums[currentColum].first;

					bool hovered = 0;
					bool clicked = 0;

					if (j.second.onlyOneEnabeled && j.second.enabeled)
					{
						if (isInButton(mousePos, transform))
						{
							hovered = true;
							if (mouseClick || mouseHeld)
							{
								internal.currentTextBox = j.first;
								clicked = true;
							}
						}


						if (j.first != internal.currentTextBox)
						{
							enabled = 0;
						}
					}

					if (enabled)
					{
						for (auto i : typedInput)
						{
							if (i == 8) //backspace
							{
								if (pos > 0)
								{
									pos--;
									text[pos] = 0;
								}
							}
							else if (i == '\n')
							{
								//ignore
							}
							else
							{
								if (pos < n - 1)
								{
									text[pos] = i;
									pos++;
									text[pos] = 0;
								}
							}
						}
					}

					if (i.second.texture.id != 0)
					{
						renderFancyBox(renderer, transform,
							i.second.colors, widget.texture, hovered, clicked);
					}

					std::string textCopy = text;

					if (j.second.displayText)
					{
						textCopy = getString(j.first) + textCopy;
					}

					if (enabled)
					{
						if ((int)timer % 2)
						{
							textCopy += "|";
						}
					}

					renderText(renderer, textCopy, font, transform, Colors_White, true,
						!hovered);


					break;
				}
				case widgetType::beginMenu:
				{

					if (drawButtonImpl())
					{
						currentMenuStack.push_back(i.first);
					};

					break;
				}

				case widgetType::startManualMenu:
				{

					currentMenuStack.push_back(i.first);

					break;
				}

				case widgetType::exitCurrentMenu:
				{
					if (currentMenuStack.size())
					{
						currentMenuStack.pop_back();
					}
					break;
				}

				case widgetType::texture:
				{

					renderTexture(renderer, colums[currentColum].first, j.second.texture, j.second.colors,
						j.second.textureCoords);

					break;
				}

				case widgetType::buttonWithTexture:
				{
					bool hovered = false;
					bool clicked = false;
					glm::vec4 transformDrawn = computeTextureNewPosition(colums[currentColum].first, j.second.texture);
					glm::vec4 aabbPos = transformDrawn;
					glm::vec4 color = j.second.colors;


					if (aabb(aabbPos, input.mousePos))
					{
						hovered = true;
						if (input.mouseHeld)
						{
							clicked = true;
							transformDrawn.y += transformDrawn.w * pressDownSize;
						}
					}

					if (hovered)
					{
						color = stepColorDown(color, 0.8);
					}

					if (input.mouseReleased && aabb(aabbPos, input.mousePos))
					{
						widget.returnFromUpdate = true;
						if (anyButtonPressed) { *anyButtonPressed = true; }
					}
					else
					{
						widget.returnFromUpdate = false;
					}

					renderTexture(renderer, transformDrawn, j.second.texture, color,
						j.second.textureCoords);

					break;
				}

				case widgetType::sliderFloatW:
				{
					if (j.second.max <= j.second.min) { break; }

					auto computedPos = colums[currentColum].first;

					glm::vec4 textTransform{computedPos.x, computedPos.y, computedPos.z / 2, computedPos.w};
					glm::vec4 sliderTransform{computedPos.x + computedPos.z / 2, computedPos.y, computedPos.z / 2, computedPos.w};

					if (MINECRAFT_LOOK_SLIDER)
					{
						textTransform = computedPos;
						sliderTransform = computedPos;
					}

					float *value = (float *)j.second.pointer;
					if (!value) { break; }

					*value = std::min(*value, j.second.max);
					*value = std::max(*value, j.second.min);


					std::string text = j.first;

					std::ostringstream s;
					s.precision(2);
					s << std::fixed << *value;

					text = getString(text) + ": " + s.str();

					if (renderSliderGeneric<float>(renderer, sliderTransform,
						value, j.second.min, j.second.max, j.second.pd.sliderBeingDragged,
						j.second.texture, j.second.colors,
						j.second.textureOver, j.second.colors2, input))
					{
						if (andSliderDragged)
						{
							*andSliderDragged = 1;
						}
					}


					renderText(renderer, text, font, textTransform, j.second.colors3,
						true);


					break;
				}

				case widgetType::colorPickerW:
				{
					auto computedPos = colums[currentColum].first;

					glm::vec4 textTransform{computedPos.x, computedPos.y, computedPos.z / 4, computedPos.w};
					glm::vec4 transform1{computedPos.x + (computedPos.z / 4.f) * 1, computedPos.y, computedPos.z / 4.f, computedPos.w};
					glm::vec4 transform2{computedPos.x + (computedPos.z / 4.f) * 2, computedPos.y, computedPos.z / 4.f, computedPos.w};
					glm::vec4 transform3{computedPos.x + (computedPos.z / 4.f) * 3, computedPos.y, computedPos.z / 4.f, computedPos.w};

					float *value;
					value = (float *)j.second.pointer;

					if (!value) { break; }

					glm::vec4 color = {value[0], value[1], value[2], 1};

					if (j.second.colors.a)
					{
						renderFancyBox(renderer, textTransform,
							color, j.second.texture, false, false);

						renderText(renderer, j.first, font, textTransform, j.second.colors, true);

					}
					else
					{
						renderText(renderer, j.first, font, textTransform, color, true);
					}

					if (j.second.colors2.a)
					{
						renderSliderFloat(renderer, transform1,
							value + 0, 0, 1, j.second.pd.sliderBeingDragged,
							j.second.texture, j.second.colors2, j.second.textureOver, {1,0,0,1}, input);

						renderSliderFloat(renderer, transform2,
							value + 1, 0, 1, j.second.pd.sliderBeingDragged2,
							j.second.texture, j.second.colors2, j.second.textureOver, {0,1,0,1}, input);

						renderSliderFloat(renderer, transform3,
							value + 2, 0, 1, j.second.pd.sliderBeingDragged3,
							j.second.texture, j.second.colors2, j.second.textureOver, {0,0,1,1}, input);

					}
					else
					{
						renderSliderFloat(renderer, transform1,
							value + 0, 0, 1, j.second.pd.sliderBeingDragged,
							j.second.texture, {1,0,0,1}, j.second.textureOver, {1,0,0,1}, input);

						renderSliderFloat(renderer, transform2,
							value + 1, 0, 1, j.second.pd.sliderBeingDragged2,
							j.second.texture, {0,1,0,1}, j.second.textureOver, {0,1,0,1}, input);

						renderSliderFloat(renderer, transform3,
							value + 2, 0, 1, j.second.pd.sliderBeingDragged3,
							j.second.texture, {0,0,1,1}, j.second.textureOver, {0,0,1,1}, input);

					}


					break;
				}
				case widgetType::newColumW:
				{
					currentColum++;
					colums[currentColum].first.y -= colums[currentColum].second; //neagate end of loop;
					break;
				}

				case widgetType::sliderUint8:
				{
					if (j.second.maxInt <= j.second.minInt) { break; }

					auto computedPos = colums[currentColum].first;

					glm::vec4 textTransform{computedPos.x, computedPos.y, computedPos.z / 2, computedPos.w};
					glm::vec4 sliderTransform{computedPos.x + computedPos.z / 2, computedPos.y, computedPos.z / 2, computedPos.w};

					if (MINECRAFT_LOOK_SLIDER)
					{
						textTransform = computedPos;
						sliderTransform = computedPos;
					}

					unsigned char *value = (unsigned char *)j.second.pointer;
					if (!value) { break; }

					*value = std::min(*value, (unsigned char)j.second.maxInt);
					*value = std::max(*value, (unsigned char)j.second.minInt);

					std::string text = j.first;

					text = getString(text) + ": " + std::to_string(*value);

					if (renderSliderGeneric<unsigned char>(renderer, sliderTransform,
						value, j.second.minInt, j.second.maxInt, j.second.pd.sliderBeingDragged,
						j.second.texture, j.second.colors, j.second.textureOver, j.second.colors2, input))
					{
						if (andSliderDragged)
						{
							*andSliderDragged = 1;
						}
					}

					renderText(renderer, text, font, textTransform, j.second.colors3, true);

					break;
				}

				case widgetType::sliderint8:
				{
					if (j.second.maxInt <= j.second.minInt) { break; }

					auto computedPos = colums[currentColum].first;

					glm::vec4 textTransform{computedPos.x, computedPos.y, computedPos.z / 2, computedPos.w};
					glm::vec4 sliderTransform{computedPos.x + computedPos.z / 2, computedPos.y, computedPos.z / 2, computedPos.w};

					if (MINECRAFT_LOOK_SLIDER)
					{
						textTransform = computedPos;
						sliderTransform = computedPos;
					}

					signed char *value = (signed char *)j.second.pointer;
					if (!value) { break; }

					*value = std::min(*value, (signed char)j.second.maxInt);
					*value = std::max(*value, (signed char)j.second.minInt);

					std::string text = j.first;

					text = getString(text) + ": " + std::to_string(*value);

					if (renderSliderGeneric<signed char>(renderer, sliderTransform,
						value, j.second.minInt, j.second.maxInt, j.second.pd.sliderBeingDragged,
						j.second.texture, j.second.colors, j.second.textureOver, j.second.colors2, input))
					{
						if (andSliderDragged)
						{
							*andSliderDragged = 1;
						}
					}

					renderText(renderer, text, font, textTransform, j.second.colors3, true);

					break;
				}

				case widgetType::sliderIntW:
				{
					if (j.second.maxInt <= j.second.minInt) { break; }

					auto computedPos = colums[currentColum].first;

					glm::vec4 textTransform{computedPos.x, computedPos.y, computedPos.z / 2, computedPos.w};
					glm::vec4 sliderTransform{computedPos.x + computedPos.z / 2, computedPos.y, computedPos.z / 2, computedPos.w};

					if (MINECRAFT_LOOK_SLIDER)
					{
						textTransform = computedPos;
						sliderTransform = computedPos;
					}

					int *value = (int *)j.second.pointer;
					if (!value) { break; }

					*value = std::min(*value, j.second.maxInt);
					*value = std::max(*value, j.second.minInt);


					std::string text = j.first;

					text = getString(text) + ": " + std::to_string(*value);

					if (renderSliderGeneric<int>(renderer, sliderTransform,
						value, j.second.minInt, j.second.maxInt, j.second.pd.sliderBeingDragged,
						j.second.texture, j.second.colors, j.second.textureOver, j.second.colors2, input))
					{
						if (andSliderDragged)
						{
							*andSliderDragged = 1;
						}
					}

					renderText(renderer, text, font, textTransform, j.second.colors3, true);

					break;
				}

				case widgetType::customWidget:
				{
					j.second.returnTransform = colums[currentColum].first;
					j.second.customWidgetUsed = true;

					j.second.hovered = aabb(j.second.returnTransform, mousePos);

					j.second.clicked = aabb(j.second.returnTransform, mousePos) && mouseClick;

					if (j.second.clicked)
					{
						if (anyCustomWidgetPressed)
						{
							*anyCustomWidgetPressed = true;
						}
					}


					break;
				}

				case widgetType::optionsToggle:
				{

					if (glui::toggleOptions(renderer, colums[currentColum].first,
						j.first, widget.colors, j.second.text2, (int *)j.second.pointer, j.second.displayText,
						font, widget.texture, widget.colors2, input.mousePos, input.mouseHeld, input.mouseReleased,
						(glm::vec4 *)j.second.pointer2, j.second.text3
						))
					{
						widget.returnFromUpdate = true;
						if (anyButtonPressed) { *anyButtonPressed = true; }
						if (anyToggleToggeled) { *anyToggleToggeled = true; }
					}
					else
					{
						widget.returnFromUpdate = false;
					}

					break;
				}

				}

				widget.justCreated = false;
				widget.lastFrameData = input;
			}

			colums[currentColum].first.y += colums[currentColum].second;
		}


		//clear unused data
		{
			std::unordered_map<std::string, Internal::Widget> widgets2;
			widgets2.reserve(internal.widgets.size());
			for (auto &i : internal.widgets)
			{
				if (i.second.usedThisFrame)
				{
					i.second.usedThisFrame = false;
					widgets2.insert(i);
				}
			}
			internal.widgets = widgets2;
		}


		renderer.currentCamera = camera;

		internal.widgetsVector.clear();

		if (!internal.idStr.empty())
		{
			errorFunc("More pushes than pops, did you forget to call End() or PopId()?");
		}
		internal.idStr.clear();

		temporalViewPort = std::nullopt;
	}

	bool RendererUi::Button(std::string name,
		const gl2d::Color4f colors, const gl2d::Texture texture)
	{
		name += internal.idStr;

		Internal::Widget widget = {};
		widget.type = widgetType::button;
		widget.colors = colors;
		widget.texture = texture;
		widget.usedThisFrame = true;
		widget.justCreated = true;
		internal.widgetsVector.push_back({name, widget});

		auto find = internal.widgets.find(name);
		if (find != internal.widgets.end())
		{
			return find->second.returnFromUpdate;
		}
		else
		{
			return false;
		}

	}

	void RendererUi::Texture(int id, gl2d::Texture t, gl2d::Color4f colors, glm::vec4 textureCoords)
	{
		Internal::Widget widget = {};
		widget.type = widgetType::texture;
		widget.texture = t;
		widget.colors = colors;
		widget.textureCoords = textureCoords;
		widget.usedThisFrame = true;
		widget.justCreated = true;

		internal.widgetsVector.push_back({"##$texture" + std::to_string(id), widget});
	}

	bool RendererUi::ButtonWithTexture(int id, gl2d::Texture t, gl2d::Color4f colors, glm::vec4 textureCoords)
	{
		Internal::Widget widget = {};
		widget.type = widgetType::buttonWithTexture;
		widget.texture = t;
		widget.colors = colors;
		widget.textureCoords = textureCoords;
		widget.usedThisFrame = true;
		widget.justCreated = true;

		std::string name = "##$textureWithId:" + std::to_string(id);

		internal.widgetsVector.push_back({name, widget});

		auto find = internal.widgets.find(name);
		if (find != internal.widgets.end())
		{
			return find->second.returnFromUpdate;
		}
		else
		{
			return false;
		}

	}

	bool RendererUi::Toggle(std::string name, const gl2d::Color4f colors, bool *toggle, const gl2d::Texture texture, const gl2d::Texture overTexture)
	{
		name += internal.idStr;

		Internal::Widget widget = {};
		widget.type = widgetType::toggle;
		widget.colors = colors;
		widget.texture = texture;
		widget.textureOver = overTexture;
		widget.usedThisFrame = true;
		widget.justCreated = true;
		widget.pointer = toggle;
		internal.widgetsVector.push_back({name, widget});

		auto find = internal.widgets.find(name);
		if (find != internal.widgets.end())
		{
			return find->second.returnFromUpdate;
		}
		else
		{
			return false;
		}

	}

	bool RendererUi::ToggleButton(std::string name, const gl2d::Color4f textColors,
		bool *toggle, const gl2d::Texture texture, const gl2d::Color4f buttonColors)
	{
		name += internal.idStr;

		Internal::Widget widget = {};
		widget.type = widgetType::toggleButton;
		widget.colors = textColors;
		widget.colors2 = buttonColors;
		widget.texture = texture;
		widget.usedThisFrame = true;
		widget.justCreated = true;
		widget.pointer = toggle;
		internal.widgetsVector.push_back({name, widget});

		auto find = internal.widgets.find(name);
		if (find != internal.widgets.end())
		{
			return find->second.returnFromUpdate;
		}
		else
		{
			return false;
		}
	}

	bool RendererUi::CustomWidget(int id, glm::vec4 *transform, bool *hovered, bool *clicked)
	{
		std::string name = "##$customWidgetWithId:" + std::to_string(id);

		Internal::Widget widget = {};
		widget.type = widgetType::customWidget;
		widget.pointer = transform;

		internal.widgetsVector.push_back({name, widget});

		auto find = internal.widgets.find(name);
		if (find != internal.widgets.end())
		{
			*transform = find->second.returnTransform;

			if (hovered)
			{
				*hovered = find->second.hovered;
				*clicked = find->second.clicked;
			}

			return find->second.customWidgetUsed;
		}
		else
		{
			*transform = {};

			return false;
		}
	}

	void RendererUi::Text(std::string name, const gl2d::Color4f colors)
	{
		name += internal.idStr;

		Internal::Widget widget = {};
		widget.type = widgetType::text;
		widget.colors = colors;
		widget.usedThisFrame = true;
		widget.justCreated = true;
		internal.widgetsVector.push_back({name, widget});
	}

	void RendererUi::newLine()
	{
		Text("", {});
	}

	void RendererUi::InputText(std::string name, char *text, size_t textSizeWithNullChar,
		gl2d::Color4f color, const gl2d::Texture texture, bool onlyOneEnabeled,
		bool displayText,
		bool enabeled)
	{
		name += internal.idStr;

		Internal::Widget widget = {};
		widget.type = widgetType::textInput;
		widget.pointer = text;
		widget.colors = color;
		widget.textSize = textSizeWithNullChar;
		widget.texture = texture;
		widget.usedThisFrame = true;
		widget.justCreated = true;
		widget.enabeled = enabeled;
		widget.displayText = displayText;
		widget.onlyOneEnabeled = onlyOneEnabeled;

		internal.widgetsVector.push_back({name, widget});
	}

	void RendererUi::sliderFloat(std::string name, float *value, float min, float max,
		gl2d::Color4f textColor,
		gl2d::Texture sliderTexture, gl2d::Color4f sliderColor,
		gl2d::Texture ballTexture, gl2d::Color4f ballColor)
	{
		name += internal.idStr;

		Internal::Widget widget = {};
		widget.type = widgetType::sliderFloatW;
		widget.pointer = value;
		widget.usedThisFrame = true;
		widget.justCreated = true;
		widget.min = min;
		widget.max = max;
		widget.colors = sliderColor;
		widget.colors2 = ballColor;
		widget.colors3 = textColor;
		widget.texture = sliderTexture;
		widget.textureOver = ballTexture;

		internal.widgetsVector.push_back({name, widget});
	}

	void RendererUi::sliderint8(std::string name, signed char *value, signed char min, signed char max,
		gl2d::Color4f textColor,
		gl2d::Texture sliderTexture, gl2d::Color4f sliderColor,
		gl2d::Texture ballTexture, gl2d::Color4f ballColor)
	{
		Internal::Widget widget = {};
		widget.type = widgetType::sliderint8;
		widget.pointer = value;
		widget.usedThisFrame = true;
		widget.justCreated = true;
		widget.minInt = min;
		widget.maxInt = max;
		widget.colors = sliderColor;
		widget.colors2 = ballColor;
		widget.colors3 = textColor;
		widget.texture = sliderTexture;
		widget.textureOver = ballTexture;

		internal.widgetsVector.push_back({name, widget});

	}

	void RendererUi::sliderUint8(std::string name, unsigned char *value, unsigned char min, unsigned char max,
		gl2d::Color4f textColor,
		gl2d::Texture sliderTexture, gl2d::Color4f sliderColor,
		gl2d::Texture ballTexture, gl2d::Color4f ballColor)
	{
		Internal::Widget widget = {};
		widget.type = widgetType::sliderUint8;
		widget.pointer = value;
		widget.usedThisFrame = true;
		widget.justCreated = true;
		widget.minInt = min;
		widget.maxInt = max;
		widget.colors = sliderColor;
		widget.colors2 = ballColor;
		widget.colors3 = textColor;
		widget.texture = sliderTexture;
		widget.textureOver = ballTexture;

		internal.widgetsVector.push_back({name, widget});

	}

	void RendererUi::sliderInt(std::string name, int *value, int min, int max,
		gl2d::Color4f textColor,
		gl2d::Texture sliderTexture, gl2d::Color4f sliderColor, gl2d::Texture ballTexture, gl2d::Color4f ballColor)
	{
		name += internal.idStr;

		Internal::Widget widget = {};
		widget.type = widgetType::sliderIntW;
		widget.pointer = value;
		widget.usedThisFrame = true;
		widget.justCreated = true;
		widget.minInt = min;
		widget.maxInt = max;
		widget.colors = sliderColor;
		widget.colors2 = ballColor;
		widget.colors3 = textColor;
		widget.texture = sliderTexture;
		widget.textureOver = ballTexture;

		internal.widgetsVector.push_back({name, widget});
	}

	void RendererUi::colorPicker(std::string name,
		float *color3Component, gl2d::Texture sliderTexture,
		gl2d::Texture ballTexture, gl2d::Color4f color
		, gl2d::Color4f color2)
	{
		Internal::Widget widget = {};
		widget.type = widgetType::colorPickerW;
		widget.pointer = color3Component;
		widget.texture = sliderTexture;
		widget.textureOver = ballTexture;
		widget.colors = color;
		widget.colors2 = color2;

		internal.widgetsVector.push_back({name, widget});

	}

	void RendererUi::toggleOptions(std::string name,
		std::string optionsSeparatedByBars,
		int *currentIndex,
		bool showText,
		gl2d::Color4f textColor,
		gl2d::Color4f *optionsColors,
		gl2d::Texture texture,
		gl2d::Color4f textureColor,
		std::string toolTip
	)
	{

		Internal::Widget widget = {};
		widget.type = widgetType::optionsToggle;
		widget.text2 = optionsSeparatedByBars;
		widget.pointer = currentIndex;
		widget.displayText = showText;
		widget.colors = textColor;
		widget.texture = texture;
		widget.colors2 = textureColor;
		widget.pointer2 = optionsColors;
		widget.text3 = toolTip;

		internal.widgetsVector.push_back({name, widget});


	}


	void RendererUi::newColum(int id)
	{
		Internal::Widget widget = {};
		widget.type = widgetType::newColumW;

		internal.widgetsVector.push_back({"##$colum" + std::to_string(id), widget});
	}

	void RendererUi::PushId(int id)
	{
		char a = *(((char *)&id) + 0);
		char b = *(((char *)&id) + 1);
		char c = *(((char *)&id) + 2);
		char d = *(((char *)&id) + 3);

		internal.idStr.push_back('#');
		internal.idStr.push_back('#');
		internal.idStr.push_back(a);
		internal.idStr.push_back(b);
		internal.idStr.push_back(c);
		internal.idStr.push_back(d);
	}

	void PushIdInternal(RendererUi &r, int id)
	{
		r.internal.idStr.push_back('#');
		r.PushId(id);
	}

	void PopIdInternal(RendererUi &r)
	{
		r.PopId();

		if (r.internal.idStr.empty())
		{
			errorFunc("More pops than pushes or inconsistent usage of begin end");
			return;
		}
		else
		{
			if (r.internal.idStr.back() != '#')
			{
				errorFunc("Inconsistent usage of begin end push pop");
				return;
			}
			r.internal.idStr.pop_back();
		}
	}

	void RendererUi::PopId()
	{
		if (internal.idStr.size() < 6)
		{
			errorFunc("More pops than pushes or inconsistent usage of begin end");
			return;
		}

		internal.idStr.pop_back();
		internal.idStr.pop_back();
		internal.idStr.pop_back();
		internal.idStr.pop_back();
		internal.idStr.pop_back();
		internal.idStr.pop_back();
	}

	int hash(const std::string &s)
	{
		unsigned int h = 0;
		int pos = 0;
		for (const auto i : s)
		{
			h += i * pos;
			pos += 1;
			pos %= 10;
		}
		return h;
	}

	void RendererUi::BeginMenu(std::string name, const gl2d::Color4f colors, const gl2d::Texture texture)
	{
		Internal::Widget widget = {};
		widget.type = widgetType::beginMenu;
		widget.colors = colors;
		widget.texture = texture;
		widget.usedThisFrame = true;
		widget.justCreated = true;
		internal.widgetsVector.push_back({name, widget});

		PushIdInternal(*this, hash(name));
	}

	void RendererUi::BeginManualMenu(std::string name)
	{
		Internal::Widget widget = {};
		widget.type = widgetType::beginManualMenu;
		internal.widgetsVector.push_back({name, widget});

		PushIdInternal(*this, hash(name));
	}

	void RendererUi::StartManualMenu(std::string name)
	{
		Internal::Widget widget = {};
		widget.type = widgetType::startManualMenu;
		internal.widgetsVector.push_back({name, widget});
	}

	void RendererUi::ExitCurrentMenu()
	{
		Internal::Widget widget = {};
		widget.type = widgetType::exitCurrentMenu;
		internal.widgetsVector.push_back({"", widget});
	}


	void RendererUi::EndMenu()
	{
		Internal::Widget widget = {};
		widget.type = widgetType::endMenu;
		widget.usedThisFrame = true;
		widget.justCreated = true;
		internal.widgetsVector.push_back({"", widget});

		PopIdInternal(*this);
	}

	//todo change ids to be unsigned and long + better hahs function

	void RendererUi::Begin(int id)
	{
		internal.alignSettings = {};

		if (!idWasSet)
		{
			idWasSet = true;
			internal.currentId = id;
		}
		else
		{
			if (internal.currentId == id)
			{
				errorFunc("Forgot to call renderFrame or more than one begin this frame");
			}
			else
			{
				errorFunc("More than one begin this frame");
			}
		}

		//will still push even if id was set so we don't get errors from inconsistent pushes
		PushIdInternal(*this, id);
	}

	void RendererUi::SetAlignModeFixedSizeWidgets(glm::ivec2 size)
	{
		internal.alignSettings.widgetSize = size;
	}


	void RendererUi::End()
	{
		PopIdInternal(*this);
	}


	//frames 

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
		if (loaded)
		{
			width = lastW;
			height = lastH;
			xPadd = lastX;
			yPadd = lastY;
		}
	}

	//todo fix frame
	glm::ivec4 Box::operator()()
	{

		if (dimensionsState == 1)
		{
			dimensions.w = dimensions.z * aspect;
		}
		else
			if (dimensionsState == 2)
			{
				dimensions.z = dimensions.w * aspect;
			}

		if (XcenterState == -1)
		{
			dimensions.x += xPadd;
		}
		if (YcenterState == -1)
		{
			dimensions.y += yPadd;
		}

		if (XcenterState == 1)
		{
			dimensions.x += xPadd + (width / 2) - (dimensions.z / 2);
		}
		if (YcenterState == 1)
		{
			dimensions.y += yPadd + (height / 2) - (dimensions.w / 2);
		}

		if (XcenterState == 2)
		{
			dimensions.x += xPadd + width - dimensions.z;
		}

		if (YcenterState == 2)
		{
			dimensions.y += yPadd + height - dimensions.w;
		}


		return dimensions;
	}

	Box &Box::xDistancePixels(int dist)
	{
		dimensions.x = dist;
		XcenterState = 0;
		return *this;
	}

	Box &Box::yDistancePixels(int dist)
	{
		dimensions.y = dist;
		YcenterState = 0;
		return *this;
	}

	Box &Box::xCenter(int dist)
	{
		dimensions.x = dist;
		XcenterState = 1;
		return *this;
	}
	Box &Box::yCenter(int dist)
	{
		dimensions.y = dist;
		YcenterState = 1;
		return *this;
	}
	Box &Box::xLeft(int dist)
	{
		dimensions.x = dist;
		XcenterState = -1;
		return *this;
	}
	Box &Box::xLeftPerc(float perc)
	{
		xLeft(perc * width);
		return *this;
	}
	Box &Box::yTop(int dist)
	{
		dimensions.y = dist;
		YcenterState = -1;
		return *this;
	}
	Box &Box::yTopPerc(float perc)
	{
		yTop(perc * height);
		return *this;
	}
	Box &Box::xRight(int dist)
	{
		dimensions.x = dist;
		XcenterState = 2;
		return *this;
	}
	Box &Box::yBottom(int dist)
	{
		dimensions.y = dist;
		YcenterState = 2;
		return *this;
	}
	Box &Box::yBottomPerc(float perc)
	{
		yBottom(perc * height);
		return *this;
	}
	Box &Box::xDimensionPixels(int dim)
	{
		dimensionsState = 0;
		dimensions.z = dim;
		return *this;
	}
	Box &Box::yDimensionPixels(int dim)
	{
		dimensionsState = 0;
		dimensions.w = dim;
		return *this;
	}
	Box &Box::xDimensionPercentage(float p)
	{
		dimensionsState = 0;
		dimensions.z = p * width;
		return *this;

	}
	Box &Box::yDimensionPercentage(float p)
	{
		dimensionsState = 0;
		dimensions.w = p * height;
		return *this;
	}
	Box &Box::xAspectRatio(float r)
	{
		dimensionsState = 2;
		aspect = r;
		return *this;
	}
	Box &Box::yAspectRatio(float r)
	{
		dimensionsState = 1;
		aspect = r;
		return *this;
	}

	bool isInButton(const glm::vec2 &p, const glm::vec4 &box)
	{
		return(p.x >= box.x && p.x <= box.x + box.z
			&&
			p.y >= box.y && p.y <= box.y + box.w
			);
	}

	glm::ivec4 Box::shrinkPercentage(glm::vec2 p)
	{
		(*this)();

		glm::vec4 b = dimensions;

		b.x += (b.z * p.x) / 2.f;
		b.y += (b.w * p.y) / 2.f;

		b.z *= (1.f - p.x);
		b.w *= (1.f - p.y);

		dimensions = b;
		return dimensions;
	}


	glm::vec4 calculateInnerPosition(glm::vec2 size, gl2d::Texture t)
	{
		glm::vec4 rez = {0,0, size.x, size.y};
		glm::ivec2 textureSize = t.GetSize();

		float textureW = textureSize.x;
		float textureH = textureSize.y;
		float w = size.x;
		float h = size.y;

		float aspect = w / h;
		float textureAspect = textureW / textureH;

		if (aspect > textureAspect)
		{
			//texture is taler than the screen, make it smaller
			float minimizeFactor = textureAspect / aspect;
			float oneMinusFactor = 1 - minimizeFactor;
			rez.x += (oneMinusFactor / 2.f) * size.x;
			rez.z -= (oneMinusFactor)*size.x;
		}
		else if (textureAspect > aspect)
		{
			//texture is longer than the screen, make it smaller
			float minimizeFactor = aspect / textureAspect;
			float oneMinusFactor = 1 - minimizeFactor;
			rez.y += (oneMinusFactor / 2.f) * size.y;
			rez.w -= (oneMinusFactor)*size.y;
		}
		else
		{
			//aspect ratio is the same
		}


		return rez;
	};

	glm::vec4 calculateInnerTextureCoords(
		glm::vec2 size, gl2d::Texture &t)
	{
		glm::ivec2 textureSize = t.GetSize();

		float textureW = textureSize.x;
		float textureH = textureSize.y;
		float w = size.x;
		float h = size.y;

		glm::vec4 textureCoords = {0,1,1,0};

		float aspect = w / h;
		float textureAspect = textureW / textureH;

		if (aspect > textureAspect)
		{
			//texture is taler than the screen, make it smaller
			float minimizeFactor = textureAspect / aspect;
			float oneMinusFactor = 1 - minimizeFactor;
			textureCoords.y = 1 - (oneMinusFactor / 2.f);
			textureCoords.w = (oneMinusFactor / 2.f);
		}
		else if (textureAspect > aspect)
		{
			//texture is longer than the screen, make it smaller
			float minimizeFactor = aspect / textureAspect;
			float oneMinusFactor = 1 - minimizeFactor;
			textureCoords.x = (oneMinusFactor / 2.f);
			textureCoords.z = 1 - (oneMinusFactor / 2.f);
		}
		else
		{
			//aspect ratio is the same
		}


		return textureCoords;
	}

};
