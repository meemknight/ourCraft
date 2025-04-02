//////////////////////////////////////////////////
//gl2d.h				1.0.3
//Copyright(c) 2023 Luta Vlad
//https://github.com/meemknight/glui
//
//
//	dependences: gl2d, glew, glm, stb_image, stb_trueType
//
// 1.0.1 added Frames and boxes
//
// 1.0.2 various stiling improvements and text
// input improvements, and toggle button
// 
// 
//////////////////////////////////////////////////


#pragma once
#include "gl2d/gl2d.h"
#include <string>
#include <unordered_map>
#include <optional>

namespace glui
{

	void defaultErrorFunc(const char *msg);
	bool aabb(glm::vec4 transform, glm::vec2 point);
	using errorFuncType = decltype(defaultErrorFunc);

	struct RendererUi
	{

		void renderFrame(
			gl2d::Renderer2D &renderer,
			gl2d::Font &font,
			glm::ivec2 mousePos,
			bool mouseClick,
			bool mouseHeld,
			bool mouseReleased,
			bool escapeReleased,
			const std::string &typedInput,
			float deltaTime
			, bool *anyButtonPressed = 0, bool *backPressed = 0,
			bool *anyCustomWidgetPressed = 0, bool *anyToggleToggeled = 0,
			bool *anyToggleDetoggeled = 0, bool *andSliderDragged = 0
		);

		bool Button(std::string name,
			const gl2d::Color4f colors, const gl2d::Texture texture = {});

		void Texture(int id, gl2d::Texture t, gl2d::Color4f colors = {1,1,1,1}, glm::vec4 textureCoords = {0,1,1,0});

		bool ButtonWithTexture(int id, gl2d::Texture t, gl2d::Color4f colors = {1,1,1,1}, glm::vec4 textureCoords = {0,1,1,0});

		bool Toggle(std::string name,
			const gl2d::Color4f colors, bool *toggle,
			const gl2d::Texture texture = {}, const gl2d::Texture overTexture = {});

		bool ToggleButton(std::string name,
			const gl2d::Color4f textColors, bool *toggle,
			const gl2d::Texture texture = {}, const gl2d::Color4f buttonColors = {1,1,1,1});


		//returns true if you should render it, clicked is optional
		bool CustomWidget(int id, glm::vec4 *transform, bool *hovered = 0, bool *clicked = 0);

		void Text(std::string name,
			const gl2d::Color4f colors);

		void newLine();

		void InputText(std::string name,
			char *text, size_t textSizeWithNullChar,
			gl2d::Color4f color = {0,0,0,0}, const gl2d::Texture texture = {},
			bool onlyOneEnabeled = 1, bool displayText = 1, bool enabeled = 1);

		void sliderFloat(std::string name, float *value, float min, float max,
			gl2d::Color4f textColor = {1,1,1,1},
			gl2d::Texture sliderTexture = {}, gl2d::Color4f sliderColor = {1,1,1,1},
			gl2d::Texture ballTexture = {}, gl2d::Color4f ballColor = {1,1,1,1});


		void sliderInt(std::string name, int *value, int min, int max,
			gl2d::Color4f textColor = {1,1,1,1},
			gl2d::Texture sliderTexture = {}, gl2d::Color4f sliderColor = {1,1,1,1},
			gl2d::Texture ballTexture = {}, gl2d::Color4f ballColor = {1,1,1,1});

		void sliderUint8(std::string name, unsigned char *value, unsigned char min, unsigned char max,
			gl2d::Color4f textColor = {1,1,1,1},
			gl2d::Texture sliderTexture = {}, gl2d::Color4f sliderColor = {1,1,1,1},
			gl2d::Texture ballTexture = {}, gl2d::Color4f ballColor = {1,1,1,1});

		void sliderint8(std::string name, signed char *value, signed char min, signed char max,
			gl2d::Color4f textColor = {1,1,1,1},
			gl2d::Texture sliderTexture = {}, gl2d::Color4f sliderColor = {1,1,1,1},
			gl2d::Texture ballTexture = {}, gl2d::Color4f ballColor = {1,1,1,1});

		void colorPicker(std::string name, float *color3Component, gl2d::Texture sliderTexture = {},
			gl2d::Texture ballTexture = {}, gl2d::Color4f color = {0,0,0,0}
		, gl2d::Color4f color2 = {0,0,0,0});

		//sepparate options by |
		void toggleOptions(std::string name,
			std::string optionsSeparatedByBars,
			int *currentIndex,
			bool showText = true,
			gl2d::Color4f textColor = {1,1,1,1},
			gl2d::Color4f *optionsColors = nullptr,
			gl2d::Texture texture = {},
			gl2d::Color4f textureColor = {1,1,1,1},
			std::string toolTip = ""
		);

		void newColum(int id);

		void PushId(int id);

		void PopId();

		void BeginMenu(std::string name, const gl2d::Color4f colors, const gl2d::Texture texture);
		void BeginManualMenu(std::string name);
		void EndMenu();

		void StartManualMenu(std::string name);
		void ExitCurrentMenu();


		void Begin(int id);
		void End();

		void SetAlignModeFixedSizeWidgets(glm::ivec2 size);

		//will automatically reset at the end of the frame!
		std::optional<glm::vec4> temporalViewPort;

		struct Internal
		{
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
				std::string text2;
				std::string text3;

				int type = 0;
				bool justCreated = true;
				bool enabeled = true;
				bool displayText = true;
				bool onlyOneEnabeled = true;
				bool usedThisFrame = 0;
				InputData lastFrameData = {};
				gl2d::Color4f colors = Colors_White;
				gl2d::Color4f colors2 = Colors_White;
				gl2d::Color4f colors3 = Colors_White;
				gl2d::Texture texture = {};
				gl2d::Texture textureOver = {};
				glm::vec4 textureCoords = {};
				bool returnFromUpdate = 0;
				bool customWidgetUsed = 0;
				void *pointer = 0;
				void *pointer2 = 0;
				bool clicked = 0; //todo for all?
				bool hovered = 0;
				float min = 0;
				float max = 0;
				int minInt = 0;
				int maxInt = 0;
				glm::vec4 returnTransform = {};//todo mabe for every widget?

				struct PersistentData
				{
					bool sliderBeingDragged = 0;
					bool sliderBeingDragged2 = 0;
					bool sliderBeingDragged3 = 0;
				}pd;

				size_t textSize = 0;
			};


			struct AlignSettings
			{
				glm::vec2 widgetSize = {};
			}alignSettings;

			std::vector<std::pair<std::string, Widget>> widgetsVector;

			std::unordered_map<std::string, Widget> widgets;

			std::unordered_map<int, std::vector<std::string>> allMenuStacks;

			std::string idStr;


			std::string currentTextBox = {};
			int currentId = 0;

		}internal;

	};

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
		Box &xLeftPerc(float perc = 0);
		Box &yTop(int dist = 0);
		Box &yTopPerc(float perc = 0);
		Box &xRight(int dist = 0);
		Box &yBottom(int dist = 0);
		Box &yBottomPerc(float perc = 0);

		Box &xDimensionPixels(int dim);
		Box &yDimensionPixels(int dim);

		Box &xDimensionPercentage(float p);
		Box &yDimensionPercentage(float p);

		Box &xAspectRatio(float r);
		Box &yAspectRatio(float r);

		glm::ivec4 operator()();

		glm::ivec4 shrinkPercentage(glm::vec2 p);

		operator glm::vec4() { return (*this)(); }
	};

	bool isInButton(const glm::vec2 &p, const glm::vec4 &box);

	float determineTextSize(gl2d::Renderer2D &renderer, const std::string &str, gl2d::Font &f, glm::vec4 transform, bool minimize);

	void renderText(gl2d::Renderer2D &renderer, const std::string &str, gl2d::Font &f, glm::vec4 transform,
		glm::vec4 color, bool noTexture, bool minimize = true, bool alignLeft = false, float maxSize = 0);

	void renderTextInput(gl2d::Renderer2D &renderer, const std::string &str,
		char *text, size_t textSizeWithNullChar, const std::string &typedInput,
		gl2d::Font &f, glm::vec4 transform, glm::vec4 colors, const gl2d::Texture texture,
		bool displayText,
		bool enabled
	);

	void renderTexture(gl2d::Renderer2D &renderer, glm::vec4 transform, gl2d::Texture t, gl2d::Color4f c, glm::vec4 textureCoordonates);

	bool renderSliderFloat(gl2d::Renderer2D &renderer, glm::vec4 transform, float *value, float min, float max, bool &sliderBeingDragged, gl2d::Texture barT, gl2d::Color4f barC, gl2d::Texture ballT, gl2d::Color4f ballC, RendererUi::Internal::InputData &input);

	bool renderSliderInt(gl2d::Renderer2D &renderer, glm::vec4 transform, int *value, int min, int max, bool &sliderBeingDragged, gl2d::Texture barT, gl2d::Color4f barC, gl2d::Texture ballT, gl2d::Color4f ballC, RendererUi::Internal::InputData &input);

	bool toggleOptions(gl2d::Renderer2D &renderer, glm::vec4 transform, const std::string &text, glm::vec4 textColor,
		const std::string &optionsSeparatedByBars, int *currentIndex, bool showText,
		gl2d::Font &font, gl2d::Texture &texture, gl2d::Color4f textureColor,
		glm::ivec2 mousePos, bool mouseHeld, bool mouseReleased, gl2d::Color4f *optionsColors = nullptr,
		std::string toolTip = "");

	bool drawButton(gl2d::Renderer2D &renderer, glm::vec4 transform, glm::vec4 color,
		const std::string &s,
		gl2d::Font &font, gl2d::Texture &texture, glm::ivec2 mousePos, bool mouseHeld, bool mouseReleased);

	glm::vec4 calculateInnerPosition(glm::vec2 size, gl2d::Texture t);

	glm::vec4 calculateInnerTextureCoords(glm::vec2 size, gl2d::Texture &t);

};