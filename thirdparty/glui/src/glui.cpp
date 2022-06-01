#include "glui/glui.h"
#include "gl2d/gl2d.h"
#include <unordered_map>
#include <iostream>

namespace glui
{


	void defaultErrorFunc(const char* msg)
	{
		std::cerr << "glui error: " << msg << "\n";
	}

	static errorFuncType* errorFunc = defaultErrorFunc;

	errorFuncType* setErrorFuncCallback(errorFuncType* newFunc)
	{
		auto a = errorFunc;
		errorFunc = newFunc;
		return a;
	}

	void gluiInit()
	{


	}

	enum widgetType
	{
		none = 0,
		button,
		toggle,
		text,
		textInput,
		beginMenu,
		endMenu,
	};

	struct InputData
	{
		glm::ivec2 mousePos = {};
		bool mouseClick = 0;
		bool mouseHeld = 0;
		bool mouseReleased = 0;
		bool escapeReleased = 0;

	};

	struct Widget
	{
		int type = 0;
		bool justCreated = true;
		bool usedThisFrame = 0;
		InputData lastFrameData = {};
		gl2d::Color4f colors = Colors_White;
		gl2d::Texture texture = {};
		gl2d::Texture textureOver = {};
		bool returnFromUpdate = 0;
		void* pointer = 0;
		size_t textSize = 0;
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

	std::vector<std::pair<std::string, Widget>> widgetsVector;

	std::unordered_map<std::string, Widget> widgets;

	std::string idStr;

	constexpr float pressDownSize = 0.04f;
	constexpr float shadowSize = 0.1f;
	constexpr float outlineSize = 0.02f;
	constexpr float textFitX = 0.95f;
	constexpr float textFitXBig = 0.98f;
	constexpr float textFitY = 0.8f;
	constexpr float textFitYBig = 0.85f;
	constexpr float buttonFit  = 0.6f;
	
	constexpr float inSizeY = 0.8;
	constexpr float inSizeX = 0.8;
	constexpr float mainInSizeX = 0.9;
	constexpr float mainInSizeY = 0.9;

	void splitTransforms(glm::vec4& down, glm::vec4& newTransform, glm::vec4 transform)
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
		glm::vec4 inversColor = glm::vec4(1, 1, 1, 1) - color;
		inversColor = inversColor * perc + color;
	
		inversColor = glm::clamp(inversColor, {0,0,0,0}, {1,1,1,1});

		return glm::vec4(glm::vec3(inversColor), color.a);
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

	void renderFancyBox(gl2d::Renderer2D& renderer, glm::vec4 transform, glm::vec4 color, gl2d::Texture t, bool hovered, bool clicked)
	{
		if (color.a <= 0.01f) { return; }

		float colorDim = 0.f;
		if (hovered)
		{
			colorDim += 0.2f;
			if (clicked)
			{
				colorDim += 0.1f;
			}
		}

		auto newColor = stepColorUp(color, colorDim);
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
			renderer.render9Patch2(transform, newColor, {}, 0.f, t, GL2D_DefaultTextureCoords, {0.2,0.8,0.8,0.2});
		
		}
	}

	glm::vec4 determineTextPos(gl2d::Renderer2D& renderer, const std::string& str, gl2d::Font& f, glm::vec4 transform,
		bool noTexture, bool minimize = true)
	{
		auto newStr = getString(str);

		float newFitX = textFitXBig;
		float newFitY = textFitYBig;

		if (minimize)
		{
			newFitX = textFitX;
			newFitY = textFitY;
		}

		glm::vec2 pos = glm::vec2(transform);

		pos.x += transform.z / 2.f;
		pos.y += transform.w / 2.f;

		float s = 1.5;
		auto size = renderer.getTextSize(newStr.c_str(), f, s);

		float newSx = s * (transform.z * newFitX) / size.x;
		float newSy = s * (transform.w * newFitY) / size.y;

		float newS = std::min(newSx, newSy);
		if (noTexture)
		{
			newS = std::max(newSx, newSy);
		}
		newS = std::min(1.f, newS);

		glm::vec2 computedSize = renderer.getTextSize(newStr.c_str(), f, newS);
		
		pos.x -= computedSize.x / 2.f;
		pos.y -= computedSize.y / 2.f;

		return glm::vec4{pos, computedSize};
	}

	//todo reuse the upper function
	void renderText(gl2d::Renderer2D& renderer,const std::string &str, gl2d::Font& f, glm::vec4 transform, glm::vec4 color, 
		bool noTexture, bool minimize = true)
	{
		auto newStr = getString(str);

		float newFitX = textFitXBig;
		float newFitY = textFitYBig;

		if (minimize)
		{
			newFitX = textFitX;
			newFitY = textFitY;
		}

		float s = 1.5;
		auto size = renderer.getTextSize(newStr.c_str(), f, s);
		float newSx = s * (transform.z * newFitX) / size.x;
		float newSy = s * (transform.w * newFitY) / size.y;

		glm::vec2 pos = glm::vec2(transform);

		pos.x += transform.z / 2.f;
		pos.y += transform.w / 2.f;
		

		float newS = std::min(newSx, newSy);
		if (noTexture)
		{
			newS = std::max(newSx, newSy);
		}

		newS = std::min(1.f, newS);

		renderer.renderText(pos, newStr.c_str(), f, color, newS);
	}


	float timer=0;
	int currentId = 0;
	bool idWasSet = 0;

	std::unordered_map<int, std::vector<std::string>> allMenuStacks;

	void renderFrame(gl2d::Renderer2D& renderer, gl2d::Font& font, glm::ivec2 mousePos, bool mouseClick,
		bool mouseHeld, bool mouseReleased, bool escapeReleased, const std::string& typedInput, float deltaTime)
	{
		if (!idWasSet)
		{
			return;
		}
		//find the menu stack for this Begin()
		auto iterMenuStack = allMenuStacks.find(currentId);
		if (iterMenuStack == allMenuStacks.end())
		{
			iterMenuStack = allMenuStacks.insert({currentId, {}}).first;
		}
		auto &currentMenuStack = iterMenuStack->second;

		idWasSet = 0;
		currentId = 0;

		if (escapeReleased && !currentMenuStack.empty())
		{
			currentMenuStack.pop_back();
		}

		timer += deltaTime*2;
		if (timer >= 2.f)
		{
			timer -= 2;
		}

		std::vector<std::pair<std::string, Widget>> widgetsCopy;
		widgetsCopy.reserve(widgetsVector.size());

		auto currentMenuStackCopy = currentMenuStack;;
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

			for (auto& i : widgetsVector)
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

		float sizeWithPaddY = ((float)renderer.windowH / widgetsCopy.size());
		float sizeY = sizeWithPaddY * inSizeY;
		float paddSizeY = sizeWithPaddY * (1 - inSizeY) / 2.f;

		float sizeWithPaddX = (float)renderer.windowW;
		float sizeX = sizeWithPaddX * inSizeX;
		float paddSizeX = sizeWithPaddX * (1 - inSizeX) / 2.f;

		glm::vec4 computedPos;
		computedPos.x = paddSizeX + (float)renderer.windowW * (1 - mainInSizeX) * 0.5f;
		computedPos.y = paddSizeY + (float)renderer.windowH * (1 - mainInSizeY) * 0.5f;
		computedPos.z = sizeX * mainInSizeX;
		computedPos.w = sizeY * mainInSizeY;

		auto camera = renderer.currentCamera;
		renderer.currentCamera.setDefault();

		InputData input;
		input.mousePos = mousePos;
		input.mouseClick = mouseClick;
		input.mouseHeld = mouseHeld;
		input.mouseReleased = mouseReleased;
		input.escapeReleased = escapeReleased;

		
		for (auto& i : widgetsCopy)
		{

			auto find = widgets.find(i.first);

			if (find == widgets.end())
			{
				
				i.second.usedThisFrame = true;
				i.second.justCreated = true;
				widgets.insert(i);
				
				
				//continue;
			}
			else
			{
				if (find->second.type != i.second.type)
				{
					errorFunc("reupdated a widget with a different type");
				}
			
				if (find->second.usedThisFrame == true)
				{
					errorFunc("used a widget name twice");
				}
				find->second.usedThisFrame = true;
				//continue;
			}

			{
				auto &j = *widgets.find(i.first);
				auto& widget = j.second;

				auto drawButton = [&]()
				{
					auto transformDrawn = computedPos;
					auto aabbTransform = computedPos;
					bool hovered = 0;
					bool clicked = 0;
					auto textColor = Colors_White;

					if (widget.colors.a <= 0.01f)
					{
						auto p = determineTextPos(renderer, j.first, font, transformDrawn, true);
						aabbTransform = p;
					}

					if (aabb(aabbTransform, input.mousePos))
					{
						hovered = true;
						if (input.mouseHeld)
						{
							clicked = true;
							transformDrawn.y += transformDrawn.w * pressDownSize;
						}
					}

					if (hovered && widget.colors.a <= 0.01f)
					{
						textColor = stepColorDown(textColor, 0.8);
					}

					if (input.mouseReleased && aabb(aabbTransform, input.mousePos))
					{
						widget.returnFromUpdate = true;
					}
					else
					{
						widget.returnFromUpdate = false;
					}

					renderFancyBox(renderer, transformDrawn, widget.colors, widget.texture, hovered, clicked);

					if ((widget.colors.a <= 0.01f || i.second.texture.id == 0))
					{
						renderText(renderer, j.first, font, transformDrawn, textColor, true, !hovered);
					}
					else
					{
						renderText(renderer, j.first, font, transformDrawn, textColor, false, hovered);
					}

					return widget.returnFromUpdate;
				};

				
				switch (widget.type)
				{
					case widgetType::button:
					{
						
						drawButton();

						break;
					}
					case widgetType::toggle:
					{
						auto transformDrawn = computedPos;
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
							*(bool*)(widget.pointer) = !(*(bool*)(widget.pointer));
						}

						widget.returnFromUpdate = *(bool*)(widget.pointer);


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
					case widgetType::text:
					{

						renderText(renderer, j.first, font, computedPos, j.second.colors, true);

						break;
					}
					case widgetType::textInput:
					{

						char* text = (char*)j.second.pointer;
						size_t n = j.second.textSize;

						int pos = strlen(text);

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

						if (i.second.texture.id != 0)
						{
							renderFancyBox(renderer, computedPos, i.second.colors, widget.texture, 0, 0);
						}
						
						std::string textCopy = text;
						if ((int)timer % 2)
						{
							textCopy += "|";
						}

						renderText(renderer, textCopy, font, computedPos, Colors_White, true);


						break;
					}
					case widgetType::beginMenu:
					{

						if (drawButton()) 
						{
							currentMenuStack.push_back(i.first);
						};

						break;
					}

				}

				widget.justCreated = false;
				widget.lastFrameData = input;
			}

			computedPos.y += (paddSizeY * 2 + sizeY) * mainInSizeY;
		}

		
		//clear unused data
		{
			std::unordered_map<std::string, Widget> widgets2;
			widgets2.reserve(widgets.size());
			for (auto& i : widgets)
			{
				if (i.second.usedThisFrame)
				{
					i.second.usedThisFrame = false;
					widgets2.insert(i);
				}
			}
			widgets = std::move(widgets2);
		}

		
		renderer.currentCamera = camera;

		widgetsVector.clear();

		if (!idStr.empty())
		{
			errorFunc("More pushes than pops");
		}
		idStr.clear();

	}

	bool Button(std::string name, const gl2d::Color4f colors, const gl2d::Texture texture)
	{
		name += idStr;

		Widget widget = {};
		widget.type = widgetType::button;
		widget.colors = colors;
		widget.texture = texture;
		widget.usedThisFrame = true;
		widget.justCreated = true;
		widgetsVector.push_back({name, widget});

		auto find = widgets.find(name);
		if (find != widgets.end())
		{
			return find->second.returnFromUpdate;
		}
		else
		{
			return false;
		}
		
	}

	bool Toggle(std::string name, const gl2d::Color4f colors, bool* toggle, const gl2d::Texture texture, const gl2d::Texture overTexture)
	{
		name += idStr;

		Widget widget = {};
		widget.type = widgetType::toggle;
		widget.colors = colors;
		widget.texture = texture;
		widget.textureOver = overTexture;
		widget.usedThisFrame = true;
		widget.justCreated = true;
		widget.pointer = toggle;
		widgetsVector.push_back({name, widget});

		auto find = widgets.find(name);
		if (find != widgets.end())
		{
			return find->second.returnFromUpdate;
		}
		else
		{
			return false;
		}

	}

	void Text(std::string name, const gl2d::Color4f colors)
	{
		name += idStr;

		Widget widget = {};
		widget.type = widgetType::text;
		widget.colors = colors;
		widget.usedThisFrame = true;
		widget.justCreated = true;
		widgetsVector.push_back({name, widget});
	}

	void InputText(std::string name, char* text, size_t textSizeWithNullChar, 
		gl2d::Color4f color, const gl2d::Texture texture)
	{
		name += idStr;

		Widget widget = {};
		widget.type = widgetType::textInput;
		widget.pointer = text;
		widget.colors = color;
		widget.textSize = textSizeWithNullChar;
		widget.texture = texture;
		widget.usedThisFrame = true;
		widget.justCreated = true;
		widgetsVector.push_back({name, widget});
	}

	void PushId(int id)
	{
		char a = *(((char*)&id) + 0);
		char b = *(((char*)&id) + 1);
		char c = *(((char*)&id) + 2);
		char d = *(((char*)&id) + 3);

		idStr.push_back('#');
		idStr.push_back('#');
		idStr.push_back(a);
		idStr.push_back(b);
		idStr.push_back(c);
		idStr.push_back(d);
	}

	void PushIdInternal(int id)
	{
		idStr.push_back('#');
		PushId(id);
	}

	void PopIdInternal()
	{
		PopId();

		if (idStr.empty())
		{
			errorFunc("More pops than pushes or inconsistent usage of begin end");
			return;
		}
		else
		{
			if (idStr.back() != '#')
			{
				errorFunc("Inconsistent usage of begin end push pop");
				return;
			}
			idStr.pop_back();
		}
	}

	void PopId()
	{
		if (idStr.size() < 6)
		{
			errorFunc("More pops than pushes or inconsistent usage of begin end");
			return;
		}

		idStr.pop_back();
		idStr.pop_back();
		idStr.pop_back();
		idStr.pop_back();
		idStr.pop_back();
		idStr.pop_back();
	}

	int hash(const std::string &s)
	{
		unsigned int h = 0;
		int pos = 0;
		for (const auto i : s)
		{
			h += i*pos;
			pos += 1;
			pos %= 10;
		}
		return h;
	}

	void BeginMenu(std::string name, const gl2d::Color4f colors, const gl2d::Texture texture)
	{
		Widget widget = {};
		widget.type = widgetType::beginMenu;
		widget.colors = colors;
		widget.texture = texture;
		widget.usedThisFrame = true;
		widget.justCreated = true;
		widgetsVector.push_back({name, widget});

		PushIdInternal(hash(name));
	}

	void EndMenu()
	{
		Widget widget = {};
		widget.type = widgetType::endMenu;
		widget.usedThisFrame = true;
		widget.justCreated = true;
		widgetsVector.push_back({"", widget});

		PopIdInternal();
	}

	//todo change ids to be unsigned and long + better hahs function

	void Begin(int id)
	{
		if (!idWasSet)
		{
			idWasSet = true;
			currentId = id;
		}
		else
		{
			if (currentId == id)
			{
				errorFunc("Forgot to call renderFrame or more than one begin this frame");
			}
			else
			{
				errorFunc("More than one begin this frame");
			}
		}

		//will still push even if id was set so we don't get errors from inconsistent pushes
		PushIdInternal(id);
	}

	void End()
	{

		PopIdInternal();
		
	}

};
