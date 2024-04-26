//////////////////////////////////////////////////
//gl2d.h				1.6.1
//Copyright(c) 2020 - 2024 Luta Vlad
//https://github.com/meemknight/gl2d
//
//	dependences: glew(or any loader you want to use), glm, stb_image, stb_trueType
//
//	features: 
//	
//	draw shapes with rotation color \
//		texture transparency
//	draw text with font and shadows
//	camera
//	custom shaders + post processing effects!
//	setVsync
//	texture atlases and loading textures with \
//		padding to fix visual bugs when using \
//		pixel art sprite atlases
//	draw to screen of frame buffer that	can \
//		be used as a texture
//
// 
//	a particle system that can use a custom \
//	shader and apply a pixelate effect
//
//
//////////////////////////////////////////////////

#pragma once

//enable simd functions
//set GL2D_SIMD to 0 if it doesn't work on your platform
#ifdef _WIN32
#define GL2D_SIMD 1
#else
#define GL2D_SIMD 0
#endif

//if you are not using visual studio make shure you link to "Opengl32.lib"

//if this is true it will use opengl130. If not it will use fome functionality from opengl3.
#define GL2D_USE_OPENGL_130 false

//this is how the library should load textures by default.
#define GL2D_DEFAULT_TEXTURE_LOAD_MODE_PIXELATED false
#define GL2D_DEFAULT_TEXTURE_LOAD_MODE_USE_MIPMAPS true


//version of the shading language.
//#version 130 is minimum, #version 330 is right now so it can run on MacOS
#define GL2D_OPNEGL_SHADER_VERSION "#version 330"
#define GL2D_OPNEGL_SHADER_PRECISION "precision highp float;"

//this is the default capacity of the renderer
#define GL2D_DefaultTextureCoords (glm::vec4{ 0, 1, 1, 0 })

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <random>
#include <stb_image/stb_image.h>
#include <stb_truetype/stb_truetype.h>
#include <vector>

namespace gl2d
{

	//Initializes the library. Call once before you use the library.
	void init();

	//Deinitializes the library.
	void clearnup();

	//The default error function, it writes to the console.
	void defaultErrorFunc(const char* msg, void *userDefinedData);

	//set by the user, it is passed to the error function
	void setUserDefinedData(void *data);

	using errorFuncType = decltype(defaultErrorFunc);

	//for the user to set a custom error function
	errorFuncType* setErrorFuncCallback(errorFuncType* newFunc);

	struct Font;

	//returns false on fail
	bool setVsync(bool b);

	///////////////////// SHADERS ///////////////////
#pragma region shaders

	struct ShaderProgram
	{
		GLuint id = 0;
		int u_sampler = 0;

		void bind() { glUseProgram(id); };

		void clear() { glDeleteProgram(id); *this = {}; }
	};

	ShaderProgram createShaderProgram(const char *vertex, const char *fragment);

	ShaderProgram createShaderFromFile(const char *filePath);

	ShaderProgram createShader(const char *fragment);

	//the only diufference between a normal shader and a post process shader,
	//is that that post process loads a vertex shader that doesn't acces color or texture uv,
	//it just renders to the entire screen, and passes the texture position to the fragment.
	ShaderProgram createPostProcessShaderFromFile(const char *filePath);

	ShaderProgram createPostProcessShader(const char *fragment);

#pragma endregion

	struct Camera;

	namespace internal
	{
		float positionToScreenCoordsX(const float position, float w);
		float positionToScreenCoordsY(const float position, float h);

		stbtt_aligned_quad fontGetGlyphQuad(const Font font, const char c);
		glm::vec4 fontGetGlyphTextureCoords(const Font font, const char c);

		glm::vec2 convertPoint(const Camera &c, const glm::vec2 &p, float windowW, float windowH);
	}

	///////////////////// COLOR ///////////////////
#pragma region Color

	using Color4f = glm::vec4;
#define Colors_Red (gl2d::Color4f{ 1, 0, 0, 1 })
#define Colors_Green (gl2d::Color4f{ 0, 1, 0, 1 })
#define Colors_Blue (gl2d::Color4f{ 0, 0, 1, 1 })
#define Colors_Black (gl2d::Color4f{ 0, 0, 0, 1 })
#define Colors_White (gl2d::Color4f{ 1, 1, 1, 1 })
#define Colors_Yellow (gl2d::Color4f{ 1, 1, 0, 1 })
#define Colors_Magenta (gl2d::Color4f{ 1, 0, 1, 1 })
#define Colors_Turqoise (gl2d::Color4f{ 0, 1, 1, 1 })
#define Colors_Orange (gl2d::Color4f{ 1, (float)0x7F / 255.0f, 0, 1 })
#define Colors_Purple (gl2d::Color4f{ 101.0f / 255.0f, 29.0f / 255.0f, 173.0f / 255.0f, 1 })
#define Colors_Gray (gl2d::Color4f{ (float)0x7F / 255.0f, (float)0x7F / 255.0f, (float)0x7F / 255.0f, 1 })
#define Colors_Transparent (gl2d::Color4f{ 0,0,0,0 })

#pragma endregion

	///////////////////// MATH ////////////////////
#pragma region math

	using Rect = glm::vec4;
	glm::vec2 rotateAroundPoint(glm::vec2 vec, glm::vec2 point, const float degrees);
	glm::vec2 scaleAroundPoint(glm::vec2 vec, glm::vec2 point, float scale);

#pragma endregion

	///////////////////// Texture /////////////////////
#pragma region Texture

	struct Texture
	{
		GLuint id = 0;

		Texture() {};
		explicit Texture(const char* file, bool pixelated = GL2D_DEFAULT_TEXTURE_LOAD_MODE_PIXELATED,
			bool useMipMaps = GL2D_DEFAULT_TEXTURE_LOAD_MODE_USE_MIPMAPS)
			{ loadFromFile(file, pixelated, useMipMaps); }

		//returns the texture dimensions
		glm::ivec2 GetSize();

		//Note: This function expects a buffer of bytes in GL_RGBA format
		void createFromBuffer(const char* image_data, const int width,
			const int height, bool pixelated = GL2D_DEFAULT_TEXTURE_LOAD_MODE_PIXELATED, bool useMipMaps = GL2D_DEFAULT_TEXTURE_LOAD_MODE_USE_MIPMAPS);
		
		//used internally. It creates a 1by1 white texture
		void create1PxSquare(const char* b = 0);
		
		void createFromFileData(const unsigned char* image_file_data, const size_t image_file_size, 
			bool pixelated = GL2D_DEFAULT_TEXTURE_LOAD_MODE_PIXELATED, bool useMipMaps = GL2D_DEFAULT_TEXTURE_LOAD_MODE_USE_MIPMAPS);

		//For texture atlases.
		//Adds a pixel padding between sprites elements to avoid some visual bugs.
		//Block size is the size of a block in pixels.
		//To be used with texture atlas padding to get the texture coordonates.
		void createFromFileDataWithPixelPadding(const unsigned char* image_file_data,
			const size_t image_file_size, int blockSize,
			bool pixelated = GL2D_DEFAULT_TEXTURE_LOAD_MODE_PIXELATED, bool useMipMaps = GL2D_DEFAULT_TEXTURE_LOAD_MODE_USE_MIPMAPS);

		void loadFromFile(const char* fileName,
			bool pixelated = GL2D_DEFAULT_TEXTURE_LOAD_MODE_PIXELATED, bool useMipMaps = GL2D_DEFAULT_TEXTURE_LOAD_MODE_USE_MIPMAPS);

		//For texture atlases.
		//Adds a pixel padding between sprites elements to avoid some visual bugs.
		//Block size is the size of a block in pixels.
		//To be used with texture atlas padding to get the texture coordonates.
		void loadFromFileWithPixelPadding(const char* fileName, int blockSize,
			bool pixelated = GL2D_DEFAULT_TEXTURE_LOAD_MODE_PIXELATED, bool useMipMaps = GL2D_DEFAULT_TEXTURE_LOAD_MODE_USE_MIPMAPS);


		//returns how much memory does the texture take (bytes),
		//used for allocating your buffer when using readTextureData
		//you can also optionally get the width and the height of the texture using outSize
		size_t getMemorySize(int mipLevel = 0, glm::ivec2 *outSize = 0);

		//reads the texture data back into RAM, you need to specify
		//the buffer to read into yourself, allocate it using
		//getMemorySize to know the size in bytes.
		//The data will be in RGBA format, one byte each component
		void readTextureData(void *buffer, int mipLevel = 0);

		//reads the texture data back into RAM
		//The data will be in RGBA format, one byte each component
		//You can also optionally get the width and the height of the texture using outSize
		std::vector<unsigned char> readTextureData(int mipLevel = 0, glm::ivec2 *outSize = 0);

		void bind(const unsigned int sample = 0);
		void unbind();

		void cleanup();
	};

#pragma endregion


	///////////////////// TextureAtlas /////////////////////
#pragma region TextureAtlas

	glm::vec4 computeTextureAtlas(int xCount, int yCount, int x, int y, bool flip = 0);

	glm::vec4 computeTextureAtlasWithPadding(int mapXsize, int mapYsize, int xCount, int yCount, int x, int y, bool flip = 0);

	//used to get the texture coordonates for a texture atlas.
	struct TextureAtlas
	{
		TextureAtlas() {};
		TextureAtlas(int x, int y) :xCount(x), yCount(y) {};

		int xCount = 0;
		int yCount = 0;

		glm::vec4 get(int x, int y, bool flip = 0)
		{
			return computeTextureAtlas(xCount, yCount, x, y, flip);
		}
	};

	//used to get the texture coordonates for a texture atlas
	//that was created using loadFromFileWithPixelPadding or createFromFileDataWithPixelPadding
	struct TextureAtlasPadding
	{
		TextureAtlasPadding() {};

		//count count size of the full texture(in pixels)
		TextureAtlasPadding(int x, int y, int xSize, int ySize) :xCount(x), yCount(y)
			, xSize(xSize), ySize(ySize)
		{
		};

		int xCount = 0;
		int yCount = 0;
		int xSize = 0;
		int ySize = 0;

		glm::vec4 get(int x, int y, bool flip = 0)
		{
			return computeTextureAtlasWithPadding(xSize, ySize, xCount, yCount, x, y, flip);
		}
	};

#pragma endregion


	///////////////////// Font /////////////////////
#pragma region Font

	//used to draw text
	struct Font
	{
		Texture           texture = {};
		glm::ivec2        size = {};
		stbtt_packedchar *packedCharsBuffer = 0;
		int               packedCharsBufferSize = 0;
		float             max_height = 0.f;

		Font() {}
		explicit Font(const char *file) { createFromFile(file); }

		void createFromTTF(const unsigned char *ttf_data, const size_t ttf_data_size);
		void createFromFile(const char *file);

		void cleanup();
	};


#pragma endregion

	///////////////////// Camera /////////////////////
#pragma region Camera

	struct Camera;

	//used to change the view.
	//whenever you render something, it will be
	//rendered relative to the current camera position.
	//so you can render 2 different things in the same frame at different camera positions.
	//(you will do that for ui for example that you will want to draw with the camera at 0 0).
	struct Camera
	{
		glm::vec2  position = {};
	
		// Camera rotation in degrees
		float rotation = 0.f;

		// Camera zoom (scaling), should be 1.0f by default
		float zoom = 1.0;

		void setDefault() { *this = Camera{}; }

		//todo not tested, add rotation
		//glm::mat3 getMatrix();

		//Used to follow objects (player for example).
		//The followed object (pos) will get placed in the center of the screen.
		//Min is the minimum distance
		//for the camera to start moving and max is the maximum possible distance.
		//w and h are the dimensions of the window
		void follow(glm::vec2 pos, float speed, float min, float max, float w, float h);
	};


#pragma endregion

	///////////////////// Renderer2d /////////////////////
#pragma region Renderer2d

	typedef glm::vec2 Position2D;
	typedef glm::vec4 Texture_Coords;

	//A franebuffer is just a texture that the user
	//can render into.
	//You can render into it using flushFBO and render the texture
	//using the texture member.
	struct FrameBuffer
	{
		FrameBuffer() {};
		explicit FrameBuffer(unsigned int w, unsigned int h) { create(w, h); };

		unsigned int fbo = 0;
		Texture texture = {};

		void create(unsigned int w, unsigned int h);
		void resize(unsigned int w, unsigned int h);

		//clears resources
		void cleanup();

		//clears colors
		void clear();
	};


	enum Renderer2DBufferType
	{
		quadPositions,
		quadColors,
		texturePositions,

		bufferSize
	};

	struct Renderer2D
	{
		Renderer2D() {};

		//feel free to delete this lines but you probably don't want to copy the renderer from a place to another
		Renderer2D(Renderer2D &other) = delete;
		Renderer2D(Renderer2D &&other) = delete;
		Renderer2D operator=(Renderer2D other) = delete;
		Renderer2D operator=(Renderer2D &other) = delete;
		Renderer2D operator=(Renderer2D &&other) = delete;

		//creates the renderer
		//fbo is the default frame buffer, 0 means drawing to the screen.
		//Quad count is the reserved quad capacity for drawing.
		//If the capacity is exceded it will be extended but this will cost performance.
		void create(GLuint fbo = 0, size_t quadCount = 1'000);

		//Clears the object alocated resources but
		//does not clear resources allocated by user like textures, fonts and fbos!
		void cleanup();

		GLuint defaultFBO = 0;

		GLuint buffers[Renderer2DBufferType::bufferSize] = {};
		GLuint vao = {};

		//4 elements each component
		std::vector<glm::vec2>spritePositions;
		std::vector<glm::vec4>spriteColors;
		std::vector<glm::vec2>texturePositions;
		std::vector<Texture>spriteTextures;
		
		//glm::vec2 spritePositions[GL2D_Renderer2D_Max_Triangle_Capacity * 6];
		//glm::vec4 spriteColors[GL2D_Renderer2D_Max_Triangle_Capacity * 6];
		//glm::vec2 texturePositions[GL2D_Renderer2D_Max_Triangle_Capacity * 6];
		//Texture   spriteTextures[GL2D_Renderer2D_Max_Triangle_Capacity * 6];

		//int spritePositionsCount = 0;
		//int spriteColorsCount = 0;
		//int texturePositionsCount = 0;
		//int spriteTexturesCount = 0;

		ShaderProgram currentShader = {};
		std::vector<ShaderProgram> shaderPushPop;
		void pushShader(ShaderProgram s = {});
		void popShader();

		Camera currentCamera = {};
		std::vector<Camera> cameraPushPop;
		void pushCamera(Camera c = {});
		void popCamera();

		glm::vec4 getViewRect(); //returns the view coordonates and size of this camera. Doesn't take rotation into account!


		//window metrics, should be up to date at all times
		int windowW = -1;
		int windowH = -1;
		void updateWindowMetrics(int w, int h) { windowW = w; windowH = h; }

		//converts pixels to screen (top left) (bottom right)
		glm::vec4 toScreen(const glm::vec4& transform);

		//clears the things that are to be drawn when calling flush
		inline void clearDrawData()
		{
			spritePositions.clear();
			spriteColors.clear();
			texturePositions.clear();
			spriteTextures.clear();

			//spritePositionsCount = 0;
			//spriteColorsCount = 0;
			//spriteTexturesCount = 0;
			//texturePositionsCount = 0;
		}

		glm::vec2 getTextSize(const char *text, const Font font, const float size = 1.5f,
			const float spacing = 4, const float line_space = 3);

		// The origin will be the bottom left corner since it represents the line for the text to be drawn
		//Pacing and lineSpace are influenced by size
		//todo the function should returns the size of the text drawn also refactor
		void renderText(glm::vec2 position, const char *text, const Font font, const Color4f color, const float size = 1.5f,
			const float spacing = 4, const float line_space = 3, bool showInCenter = 1, const Color4f ShadowColor = {0.1,0.1,0.1,1}
		, const Color4f LightColor = {});

		//determines the text size so that it fits in the given box,
		//the x and y components of the transform are ignored
		float determineTextRescaleFitSmaller(const std::string &str,
			gl2d::Font &f, glm::vec4 transform, float maxSize);

		//determines the text size so that it fits in the given box,
		//the x and y components of the transform are ignored
		float determineTextRescaleFit(const std::string &str,
			gl2d::Font &f, glm::vec4 transform);

		//returns number of lines
		//out rez is optional
		int wrap(const std::string &in, gl2d::Font &f,
			float baseSize, float maxDimension, std::string *outRez);

		// The origin will be the bottom left corner since it represents the line for the text to be drawn
		//Pacing and lineSpace are influenced by size
		//todo the function should returns the size of the text drawn also refactor
		void renderTextWrapped(const std::string &text,
			gl2d::Font f, glm::vec4 textPos, glm::vec4 color, float baseSize,
			float spacing = 4, float lineSpacing = 3,
			bool showInCenter = true, glm::vec4 shadowColor = {0.1,0.1,0.1,1}, glm::vec4 lightColor = {});

		glm::vec2 getTextSizeWrapped(const std::string &text,
			gl2d::Font f, float maxTextLenght, float baseSize, float spacing = 4, float lineSpacing = 3);

		//determines the text size so that it fits in the given box,
		//the x and y components of the transform are ignored
		float determineTextRescaleFitBigger(const std::string &str,
			gl2d::Font &f, glm::vec4 transform, float minSize);

		void renderRectangle(const Rect transforms, const Texture texture, const Color4f colors[4], const glm::vec2 origin = {}, const float rotationDegrees = 0.f, const glm::vec4 textureCoords = GL2D_DefaultTextureCoords);
		inline void renderRectangle(const Rect transforms, const Texture texture, const Color4f colors = {1,1,1,1}, const glm::vec2 origin = {}, const float rotationDegrees = 0, const glm::vec4 textureCoords = GL2D_DefaultTextureCoords)
		{
			Color4f c[4] = { colors,colors,colors,colors };
			renderRectangle(transforms, texture, c, origin, rotationDegrees, textureCoords);
		}

		//abs rotation means that the rotaion is relative to the screen rather than object
		void renderRectangleAbsRotation(const Rect transforms, const Texture texture, const Color4f colors[4], const glm::vec2 origin = {}, const float rotationDegrees = 0.f, const glm::vec4 textureCoords = GL2D_DefaultTextureCoords);
		inline void renderRectangleAbsRotation(const Rect transforms, const Texture texture, const Color4f colors = {1,1,1,1}, const glm::vec2 origin = {}, const float rotationDegrees = 0.f, const glm::vec4 textureCoords = GL2D_DefaultTextureCoords)
		{
			Color4f c[4] = { colors,colors,colors,colors };
			renderRectangleAbsRotation(transforms, texture, c, origin, rotationDegrees, textureCoords);
		}

		void renderRectangle(const Rect transforms, const Color4f colors[4], const glm::vec2 origin = { 0,0 }, const float rotationDegrees = 0);
		inline void renderRectangle(const Rect transforms, const Color4f colors = {1,1,1,1}, const glm::vec2 origin = {0,0}, const float rotationDegrees = 0)
		{
			Color4f c[4] = { colors,colors,colors,colors };
			renderRectangle(transforms, c, origin, rotationDegrees);
		}

		//abs rotation means that the rotaion is relative to the screen rather than object
		void renderRectangleAbsRotation(const Rect transforms, const Color4f colors[4], const glm::vec2 origin = { 0,0 }, const float rotationDegrees = 0);
		inline void renderRectangleAbsRotation(const Rect transforms, const Color4f colors = {1,1,1,1}, const glm::vec2 origin = { 0,0 }, const float rotationDegrees = 0)
		{
			Color4f c[4] = { colors,colors,colors,colors };
			renderRectangleAbsRotation(transforms, c, origin, rotationDegrees);
		}

		void renderLine(const glm::vec2 position, const float angleDegrees, const float length, const Color4f color, const float width = 2.f);

		void renderLine(const glm::vec2 start, const glm::vec2 end, const Color4f color, const float width = 2.f);

		void renderRectangleOutline(const glm::vec4 position, const Color4f color, const float width = 2.f, const glm::vec2 origin = {}, const float rotationDegrees = 0);
		
		void renderCircleOutline(const glm::vec2 position, const Color4f color, const float size, const float width = 2.f, const unsigned int segments = 16);

		//legacy, use render9Patch2
		void render9Patch(const Rect position, const int borderSize, const Color4f color, const glm::vec2 origin, const float rotationDegrees, const Texture texture, const Texture_Coords textureCoords, const Texture_Coords inner_texture_coords);

		//used for ui. draws a texture that scales the margins different so buttons of different sizes can be drawn.
		void render9Patch2(const Rect position, const Color4f color, const glm::vec2 origin, const float rotationDegrees, const Texture texture, const Texture_Coords textureCoords, const Texture_Coords inner_texture_coords);

		void clearScreen(const Color4f color = Color4f{0,0,0,0});

		void setShaderProgram(const ShaderProgram shader);
		void setCamera(const Camera camera);

		//will reset on the current stack
		void resetCameraAndShader();

		//The framebuffer need to have the same size as the input!
		void renderPostProcess(ShaderProgram shader, Texture input, FrameBuffer result = {});

		//Only when this function is called it draws to the screen the things rendered.
		//If clearDrawData is false, the rendering information will be kept.
		//Usefull if you want to render something twice or render again on top for some reason
		void flush(bool clearDrawData = true);

		//Renders to a fbo instead of the screen. The fbo is just a texture.
		//If clearDrawData is false, the rendering information will be kept.
		void flushFBO(FrameBuffer frameBuffer, bool clearDrawData = true);

		void renderFrameBufferToTheEntireScreen(gl2d::FrameBuffer fbo, gl2d::FrameBuffer screen = {});

		void renderTextureToTheEntireScreen(gl2d::Texture t, gl2d::FrameBuffer screen = {});

		gl2d::FrameBuffer postProcessFbo1 = {};
		gl2d::FrameBuffer postProcessFbo2 = {};
		
		//internal use
		bool internalPostProcessFlip = 0;

		//the FBO size should be equal to the current configured w and h of the renderer
		void flushPostProcess(const std::vector<ShaderProgram> &postProcesses, 
			FrameBuffer frameBuffer = {}, bool clearDrawData = true);

		//the FBO size should be equal to the current configured w and h of the renderer
		void postProcessOverATexture(const std::vector<ShaderProgram> &postProcesses,
			gl2d::Texture in,
			FrameBuffer frameBuffer = {});
	};

	void enableNecessaryGLFeatures();

#pragma endregion






};