//////////////////////////////////////////////////
//gl2d.cpp				1.5.2
//Copyright(c) 2020 - 2024 Luta Vlad
//https://github.com/meemknight/gl2d
// 
//notes: 
// 1.2.1
// fixed alpha in the particle system with the 
// post process
// 
// 1.2.2
// added default values to structs
// added some more error reporting
// added option to change gl version and 
//  shader presision
// vsynk independend of gl loader
// 
// 1.2.3
// small problems fixes
// texture load flags
// working on 9pathces
// 
// 1.2.4
// working at fixing get text size
// 
// 1.2.5
// push pop shaders and camera
// added getViewRect
// 
// 1.2.6
// updated camera.follow
// removed TextureRegion
// 
// 1.3.0
// polished using custom shader api
// fixed camera follow
// moved the particle system into another file
// added a proper cmake
// used the proper stbi free function
// added a default fbo support
// added proper error reporting (with uer defined data)
// 
// 1.4.0
// much needed api refactoring
// removed capacity render limit
// added some more comments
// 
// 1.4.1
// line rendering
// rect outline rendering
// circle outline rendering
// 
// 1.5.0
// started to add some more needed text functions
// needed to be tested tho
// 
// 1.5.1
// fixed the follow function
// 
// 1.5.2
// read texture data + report error if opengl not loaded
// 
/////////////////////////////////////////////////////////


//	todo
//
//	add particle demo
//	shaders demo
//	add matrices transforms
//	flags for vbos
//	
//

#include <gl2d/gl2d.h>

#ifdef _WIN32
#include <Windows.h>
#endif

#include <fstream>
#include <sstream>
#include <algorithm>
#include <iostream>

//if you are not using visual studio make shure you link to "Opengl32.lib"
#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4244 4305 4267 4996 4018)
#pragma comment(lib, "Opengl32.lib")
#endif

#undef max


namespace gl2d
{
#pragma region shaders

	static ShaderProgram defaultShader = {};
	static Camera defaultCamera{};
	static Texture white1pxSquareTexture = {};

	static const char* defaultVertexShader =
		GL2D_OPNEGL_SHADER_VERSION "\n"
		GL2D_OPNEGL_SHADER_PRECISION "\n"
		"in vec2 quad_positions;\n"
		"in vec4 quad_colors;\n"
		"in vec2 texturePositions;\n"
		"out vec4 v_color;\n"
		"out vec2 v_texture;\n"
		"void main()\n"
		"{\n"
		"	gl_Position = vec4(quad_positions, 0, 1);\n"
		"	v_color = quad_colors;\n"
		"	v_texture = texturePositions;\n"
		"}\n";

	static const char* defaultFragmentShader =
		GL2D_OPNEGL_SHADER_VERSION "\n"
		GL2D_OPNEGL_SHADER_PRECISION "\n"
		"out vec4 color;\n"
		"in vec4 v_color;\n"
		"in vec2 v_texture;\n"
		"uniform sampler2D u_sampler;\n"
		"void main()\n"
		"{\n"
		"    color = v_color * texture2D(u_sampler, v_texture);\n"
		"}\n";

#pragma endregion

	static errorFuncType* errorFunc = defaultErrorFunc;

	void defaultErrorFunc(const char* msg, void *userDefinedData)
	{
		std::cerr << "gl2d error: " << msg << "\n";
	}

	void *userDefinedData = 0;
	void setUserDefinedData(void *data)
	{
		userDefinedData = data;
	}

	errorFuncType* setErrorFuncCallback(errorFuncType* newFunc)
	{
		auto a = errorFunc;
		errorFunc = newFunc;
		return a;
	}

	namespace internal
	{
		float positionToScreenCoordsX(const float position, float w)
		{
			return (position / w) * 2 - 1;
		}

		float positionToScreenCoordsY(const float position, float h)
		{
			return -((-position / h) * 2 - 1);
		}

		stbtt_aligned_quad fontGetGlyphQuad(const Font font, const char c)
		{
			stbtt_aligned_quad quad = {0};

			float x = 0;
			float y = 0;

			stbtt_GetPackedQuad(font.packedCharsBuffer,
				font.size.x, font.size.y, c - ' ', &x, &y, &quad, 1);


			return quad;
		}

		glm::vec4 fontGetGlyphTextureCoords(const Font font, const char c)
		{
			float xoffset = 0;
			float yoffset = 0;

			const stbtt_aligned_quad quad = fontGetGlyphQuad(font, c);

			return glm::vec4{quad.s0, quad.t0, quad.s1, quad.t1};
		}

		GLuint loadShader(const char* source, GLenum shaderType)
		{
			GLuint id = glCreateShader(shaderType);

			glShaderSource(id, 1, &source, 0);
			glCompileShader(id);

			int result = 0;
			glGetShaderiv(id, GL_COMPILE_STATUS, &result);

			if (!result)
			{
				char* message = 0;
				int   l = 0;

				glGetShaderiv(id, GL_INFO_LOG_LENGTH, &l);

				message = new char[l];

				glGetShaderInfoLog(id, l, &l, message);

				message[l - 1] = 0;

				errorFunc(message, userDefinedData);

				delete[] message;

			}

			return id;
		}
		
	}

#ifdef _WIN32
	typedef BOOL(WINAPI* PFNWGLSWAPINTERVALEXTPROC) (int interval);
#else
	typedef bool(*PFNWGLSWAPINTERVALEXTPROC) (int interval);
#endif

	struct
	{
		PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT;
	}extensions = {};

	bool hasInitialized = 0;
	void init()
	{
		if (hasInitialized) { return; }
		hasInitialized = true;

		if (!glGenTextures)
		{
			errorFunc("OpenGL doesn't \
seem to be initialized, have you forgotten to call gladLoadGL() \
or gladLoadGLLoader() or glewInit()?", userDefinedData);
		}


		//int last = 0;
		//glGetIntegerv(GL_NUM_EXTENSIONS, &last);
		//for(int i=0; i<last; i++)
		//{
		//	const char *c = (const char*)glGetStringi(GL_EXTENSIONS, i);
		//	if(strcmp(c, "WGL_EXT_swap_control") == 0)
		//	{
		//		extensions.WGL_EXT_swap_control_ext = true;
		//		break;
		//	}
		//}

	#ifdef _WIN32
		//add linux suport
		
		//if you are not using visual studio make shure you link to "Opengl32.lib"
		extensions.wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
	#endif

		defaultShader = createShaderProgram(defaultVertexShader, defaultFragmentShader);
		white1pxSquareTexture.create1PxSquare();

		enableNecessaryGLFeatures();
	}

	void clearnup()
	{
		white1pxSquareTexture.cleanup();
		glDeleteShader(defaultShader.id);
		hasInitialized = false;
	}

	bool setVsync(bool b)
	{
		//add linux suport
		if (extensions.wglSwapIntervalEXT != nullptr)
		{
			bool rezult = extensions.wglSwapIntervalEXT(b);
			return rezult;
		}
		else
		{
			return false;
		}
	}

	glm::vec2 rotateAroundPoint(glm::vec2 vec, glm::vec2 point, const float degrees)
	{
		point.y = -point.y;
		float a = glm::radians(degrees);
		float s = sinf(a);
		float c = cosf(a);
		vec.x -= point.x;
		vec.y -= point.y;
		float newx = vec.x * c - vec.y * s;
		float newy = vec.x * s + vec.y * c;
		// translate point back:
		vec.x = newx + point.x;
		vec.y = newy + point.y;
		return vec;
	}

	glm::vec2 scaleAroundPoint(glm::vec2 vec, glm::vec2 point, float scale)
	{
		vec = (vec - point) * scale + point;

		return vec;
	}


	///////////////////// Shader /////////////////////
#pragma region shader

	ShaderProgram createShaderProgram(const char *vertex, const char *fragment)
	{
		ShaderProgram shader = {0};

		const GLuint vertexId = internal::loadShader(vertex, GL_VERTEX_SHADER);
		const GLuint fragmentId = internal::loadShader(fragment, GL_FRAGMENT_SHADER);

		shader.id = glCreateProgram();
		glAttachShader(shader.id, vertexId);
		glAttachShader(shader.id, fragmentId);

		glBindAttribLocation(shader.id, 0, "quad_positions");
		glBindAttribLocation(shader.id, 1, "quad_colors");
		glBindAttribLocation(shader.id, 2, "texturePositions");

		glLinkProgram(shader.id);

		glDeleteShader(vertexId);
		glDeleteShader(fragmentId);

		int info = 0;
		glGetProgramiv(shader.id, GL_LINK_STATUS, &info);

		if (info != GL_TRUE)
		{
			char *message = 0;
			int   l = 0;

			glGetProgramiv(shader.id, GL_INFO_LOG_LENGTH, &l);

			message = new char[l];

			glGetProgramInfoLog(shader.id, l, &l, message);

			errorFunc(message, userDefinedData);

			delete[] message;
		}

		glValidateProgram(shader.id);

		shader.u_sampler = glGetUniformLocation(shader.id, "u_sampler");

		return shader;
	}

#pragma endregion

	///////////////////// Texture /////////////////////
#pragma region Texture

	void convertFromRetardedCoordonates(int tSizeX, int tSizeY, int x, int y, int sizeX, int sizeY, int s1, int s2, int s3, int s4, Texture_Coords* outer, Texture_Coords* inner)
	{
		float newX = (float)tSizeX / (float)x;
		float newY = (float)tSizeY / (float)y;
		newY = 1 - newY;

		float newSizeX = (float)tSizeX / (float)sizeX;
		float newSizeY = (float)tSizeY / (float)sizeY;

		if (outer)
		{
			outer->x = newX;
			outer->y = newY;
			outer->z = newX + newSizeX;
			outer->w = newY - newSizeY;
		}

		if (inner)
		{
			inner->x = newX + ((float)s1 / tSizeX);
			inner->y = newY - ((float)s2 / tSizeY);
			inner->z = newX + newSizeX - ((float)s3 / tSizeX);
			inner->w = newY - newSizeY + ((float)s4 / tSizeY);
		}

	}

#pragma endregion

	///////////////////// Font /////////////////////
#pragma	region Font

	void Font::createFromTTF(const unsigned char *ttf_data, const size_t ttf_data_size)
	{
		size.x = 2000,
		size.y = 2000,
		max_height = 0,
		packedCharsBufferSize = ('~' - ' ');

		//STB TrueType will give us a one channel buffer of the font that we then convert to RGBA for OpenGL
		const size_t fontMonochromeBufferSize = size.x * size.y;
		const size_t fontRgbaBufferSize = size.x * size.y * 4;

		unsigned char *fontMonochromeBuffer = new unsigned char[fontMonochromeBufferSize];
		unsigned char *fontRgbaBuffer = new unsigned char[fontRgbaBufferSize];

		packedCharsBuffer = new stbtt_packedchar[packedCharsBufferSize]{};

		stbtt_pack_context stbtt_context;
		stbtt_PackBegin(&stbtt_context, fontMonochromeBuffer, size.x, size.y, 0, 2, NULL);
		stbtt_PackSetOversampling(&stbtt_context, 2, 2);
		stbtt_PackFontRange(&stbtt_context, ttf_data, 0, 65, ' ', '~' - ' ', packedCharsBuffer);
		stbtt_PackEnd(&stbtt_context);

		for (int i = 0; i < fontMonochromeBufferSize; i++)
		{

			fontRgbaBuffer[(i * 4)] = fontMonochromeBuffer[i];
			fontRgbaBuffer[(i * 4) + 1] = fontMonochromeBuffer[i];
			fontRgbaBuffer[(i * 4) + 2] = fontMonochromeBuffer[i];

			if (fontMonochromeBuffer[i] > 1)
			{
				fontRgbaBuffer[(i * 4) + 3] = 255;
			}
			else
			{
				fontRgbaBuffer[(i * 4) + 3] = 0;
			}
		}

		//Init texture
		{
			glGenTextures(1, &texture.id);
			glBindTexture(GL_TEXTURE_2D, texture.id);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, fontRgbaBuffer);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		}

		delete[] fontMonochromeBuffer;
		delete[] fontRgbaBuffer;

		for (char c = ' '; c <= '~'; c++)
		{
			const stbtt_aligned_quad  q = internal::fontGetGlyphQuad(*this, c);
			const float               m = q.y1 - q.y0;

			if (m > max_height && m < 1.e+8f)
			{
				max_height = m;
			}
		}

	}

	void Font::createFromFile(const char *file)
	{
		std::ifstream fileFont(file, std::ios::binary);

		if (!fileFont.is_open())
		{
			char c[300] = {0};
			strcat(c, "error openning: ");
			strcat(c + strlen(c), file);
			errorFunc(c, userDefinedData);
			return;
		}

		int fileSize = 0;
		fileFont.seekg(0, std::ios::end);
		fileSize = (int)fileFont.tellg();
		fileFont.seekg(0, std::ios::beg);
		unsigned char *fileData = new unsigned char[fileSize];
		fileFont.read((char *)fileData, fileSize);
		fileFont.close();

		createFromTTF(fileData, fileSize);

		delete[] fileData;
	}

	void Font::cleanup()
	{
		texture.cleanup();
		*this = {};
	}


#pragma endregion

	///////////////////// Camera /////////////////////
#pragma region Camera

#pragma endregion

	///////////////////// Renderer2D /////////////////////
#pragma region Renderer2D

	//won't bind any fbo
	void internalFlush(gl2d::Renderer2D &renderer, bool clearDrawData)
	{
		enableNecessaryGLFeatures();

		if (!hasInitialized)
		{
			errorFunc("Library not initialized. Have you forgotten to call gl2d::init() ?", userDefinedData);
		}

		if (!renderer.vao)
		{
			errorFunc("Renderer not initialized. Have you forgotten to call gl2d::Renderer2D::create() ?", userDefinedData);
		}

		if (renderer.windowH == 0 || renderer.windowW == 0)
		{
			if (clearDrawData)
			{
				renderer.clearDrawData();
			}

			return;
		}

		if (renderer.windowH < 0 || renderer.windowW < 0)
		{
			if (clearDrawData)
			{
				renderer.clearDrawData();
			}

			errorFunc("Negative windowW or windowH, have you forgotten to call updateWindowMetrics(w, h)?", userDefinedData);

			return;
		}

		if(renderer.spriteTextures.empty())
		{
			return;
		}

		glViewport(0, 0, renderer.windowW, renderer.windowH);

		glBindVertexArray(renderer.vao);

		glUseProgram(renderer.currentShader.id);

		glUniform1i(renderer.currentShader.u_sampler, 0);

		glBindBuffer(GL_ARRAY_BUFFER, renderer.buffers[Renderer2DBufferType::quadPositions]);
		glBufferData(GL_ARRAY_BUFFER, renderer.spritePositions.size() * sizeof(glm::vec2), renderer.spritePositions.data(), GL_STREAM_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, renderer.buffers[Renderer2DBufferType::quadColors]);
		glBufferData(GL_ARRAY_BUFFER, renderer.spriteColors.size() * sizeof(glm::vec4), renderer.spriteColors.data(), GL_STREAM_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, renderer.buffers[Renderer2DBufferType::texturePositions]);
		glBufferData(GL_ARRAY_BUFFER, renderer.texturePositions.size() * sizeof(glm::vec2), renderer.texturePositions.data(), GL_STREAM_DRAW);

		//Instance render the textures
		{
			const int size = renderer.spriteTextures.size();
			int pos = 0;
			unsigned int id = renderer.spriteTextures[0].id;

			renderer.spriteTextures[0].bind();

			for (int i = 1; i < size; i++)
			{
				if (renderer.spriteTextures[i].id != id)
				{
					glDrawArrays(GL_TRIANGLES, pos * 6, 6 * (i - pos));

					pos = i;
					id = renderer.spriteTextures[i].id;

					renderer.spriteTextures[i].bind();
				}

			}

			glDrawArrays(GL_TRIANGLES, pos * 6, 6 * (size - pos));

			glBindVertexArray(0);
		}

		if (clearDrawData) 
		{
			renderer.clearDrawData();
		}
	}

	void gl2d::Renderer2D::flush(bool clearDrawData)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, defaultFBO);
		internalFlush(*this, clearDrawData);
	}

	void Renderer2D::flushFBO(FrameBuffer frameBuffer, bool clearDrawData)
	{
		if (frameBuffer.fbo == 0) 
		{
			errorFunc("Framebuffer not initialized", userDefinedData);
		}

		glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer.fbo);
		glBindTexture(GL_TEXTURE_2D, 0); //todo investigate and remove

		internalFlush(*this, clearDrawData);

		glBindFramebuffer(GL_FRAMEBUFFER, defaultFBO);
	}

	void enableNecessaryGLFeatures()
	{
		glEnable(GL_BLEND);
		glDisable(GL_DEPTH_TEST);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	}

	///////////////////// Renderer2D - render ///////////////////// 

	void Renderer2D::renderRectangle(const Rect transforms, const Texture texture, const Color4f colors[4], const glm::vec2 origin, const float rotation, const glm::vec4 textureCoords)
	{
		glm::vec2 newOrigin;
		newOrigin.x = origin.x + transforms.x + (transforms.z / 2);
		newOrigin.y = origin.y + transforms.y + (transforms.w / 2);
		renderRectangleAbsRotation(transforms, texture, colors, newOrigin, rotation, textureCoords);
	}

	void gl2d::Renderer2D::renderRectangleAbsRotation(const Rect transforms, 
		const Texture texture, const Color4f colors[4], const glm::vec2 origin, const float rotation, const glm::vec4 textureCoords)
	{
		Texture textureCopy = texture;

		if (textureCopy.id == 0)
		{
			errorFunc("Invalid texture", userDefinedData);
			textureCopy = white1pxSquareTexture;
		}

		//We need to flip texture_transforms.y
		const float transformsY = transforms.y * -1;

		glm::vec2 v1 = { transforms.x,				  transformsY };
		glm::vec2 v2 = { transforms.x,				  transformsY - transforms.w };
		glm::vec2 v3 = { transforms.x + transforms.z, transformsY - transforms.w };
		glm::vec2 v4 = { transforms.x + transforms.z, transformsY };

		//Apply rotations
		if (rotation != 0)
		{
			v1 = rotateAroundPoint(v1, origin, rotation);
			v2 = rotateAroundPoint(v2, origin, rotation);
			v3 = rotateAroundPoint(v3, origin, rotation);
			v4 = rotateAroundPoint(v4, origin, rotation);
		}

		//Apply camera transformations
		v1.x -= currentCamera.position.x;
		v1.y += currentCamera.position.y;
		v2.x -= currentCamera.position.x;
		v2.y += currentCamera.position.y;
		v3.x -= currentCamera.position.x;
		v3.y += currentCamera.position.y;
		v4.x -= currentCamera.position.x;
		v4.y += currentCamera.position.y;

		//Apply camera rotation
		if (currentCamera.rotation != 0)
		{
			glm::vec2 cameraCenter;

			cameraCenter.x = windowW / 2.0f;
			cameraCenter.y = windowH / 2.0f;

			v1 = rotateAroundPoint(v1, cameraCenter, currentCamera.rotation);
			v2 = rotateAroundPoint(v2, cameraCenter, currentCamera.rotation);
			v3 = rotateAroundPoint(v3, cameraCenter, currentCamera.rotation);
			v4 = rotateAroundPoint(v4, cameraCenter, currentCamera.rotation);
		}

		//Apply camera zoom
		//if(renderer->currentCamera.zoom != 1)
		{

			glm::vec2 cameraCenter;
			cameraCenter.x = windowW / 2.0f;
			cameraCenter.y = -windowH / 2.0f;

			v1 = scaleAroundPoint(v1, cameraCenter, currentCamera.zoom);
			v2 = scaleAroundPoint(v2, cameraCenter, currentCamera.zoom);
			v3 = scaleAroundPoint(v3, cameraCenter, currentCamera.zoom);
			v4 = scaleAroundPoint(v4, cameraCenter, currentCamera.zoom);
		}

		v1.x = internal::positionToScreenCoordsX(v1.x, (float)windowW);
		v2.x = internal::positionToScreenCoordsX(v2.x, (float)windowW);
		v3.x = internal::positionToScreenCoordsX(v3.x, (float)windowW);
		v4.x = internal::positionToScreenCoordsX(v4.x, (float)windowW);
		v1.y = internal::positionToScreenCoordsY(v1.y, (float)windowH);
		v2.y = internal::positionToScreenCoordsY(v2.y, (float)windowH);
		v3.y = internal::positionToScreenCoordsY(v3.y, (float)windowH);
		v4.y = internal::positionToScreenCoordsY(v4.y, (float)windowH);

		spritePositions.push_back(glm::vec2{ v1.x, v1.y });
		spritePositions.push_back(glm::vec2{ v2.x, v2.y });
		spritePositions.push_back(glm::vec2{ v4.x, v4.y });

		spritePositions.push_back(glm::vec2{ v2.x, v2.y });
		spritePositions.push_back(glm::vec2{ v3.x, v3.y });
		spritePositions.push_back(glm::vec2{ v4.x, v4.y });

		spriteColors.push_back(colors[0]);
		spriteColors.push_back(colors[1]);
		spriteColors.push_back(colors[3]);
		spriteColors.push_back(colors[1]);
		spriteColors.push_back(colors[2]);
		spriteColors.push_back(colors[3]);

		texturePositions.push_back(glm::vec2{ textureCoords.x, textureCoords.y }); //1
		texturePositions.push_back(glm::vec2{ textureCoords.x, textureCoords.w }); //2
		texturePositions.push_back(glm::vec2{ textureCoords.z, textureCoords.y }); //4
		texturePositions.push_back(glm::vec2{ textureCoords.x, textureCoords.w }); //2
		texturePositions.push_back(glm::vec2{ textureCoords.z, textureCoords.w }); //3
		texturePositions.push_back(glm::vec2{ textureCoords.z, textureCoords.y }); //4

		spriteTextures.push_back(textureCopy);
	}

	void Renderer2D::renderRectangle(const Rect transforms, const Color4f colors[4], const glm::vec2 origin, const float rotation)
	{
		renderRectangle(transforms, white1pxSquareTexture, colors, origin, rotation);
	}

	void Renderer2D::renderRectangleAbsRotation(const Rect transforms, const Color4f colors[4], const glm::vec2 origin, const float rotation)
	{
		renderRectangleAbsRotation(transforms, white1pxSquareTexture, colors, origin, rotation);
	}

	void Renderer2D::renderLine(const glm::vec2 position, const float angleDegrees, const float length, const Color4f color, const float width)
	{
		renderRectangle({position - glm::vec2(0,width / 2.f), length, width},
			color, {-length/2, 0}, angleDegrees);
	}

	void Renderer2D::renderLine(const glm::vec2 start, const glm::vec2 end, const Color4f color, const float width) 
	{
		glm::vec2 vector = end - start;
		float length = glm::length(vector);
		float angle = std::atan2(vector.y, vector.x);
		renderLine(start, -glm::degrees(angle), length, color, width);
	}

	void Renderer2D::renderRectangleOutline(const glm::vec4 position, const Color4f color, const float width,
		const glm::vec2 origin, const float rotationDegrees)
	{
		
		glm::vec2 topLeft = position;
		glm::vec2 topRight = glm::vec2(position) + glm::vec2(position.z, 0);
		glm::vec2 bottomLeft = glm::vec2(position) + glm::vec2(0, position.w);
		glm::vec2 bottomRight = glm::vec2(position) + glm::vec2(position.z, position.w);
		
		glm::vec2 p1 = topLeft + glm::vec2(-width / 2.f, 0);
		glm::vec2 p2 = topRight + glm::vec2(+width / 2.f, 0);
		glm::vec2 p3 = topRight + glm::vec2(0, +width / 2.f);
		glm::vec2 p4 = bottomRight + glm::vec2(0, -width / 2.f);
		glm::vec2 p5 = bottomRight + glm::vec2(width / 2.f, 0);
		glm::vec2 p6 = bottomLeft + glm::vec2(-width / 2.f, 0);
		glm::vec2 p7 = bottomLeft + glm::vec2(0, -width / 2.f);
		glm::vec2 p8 = topLeft + glm::vec2(0, +width / 2.f);

		if (rotationDegrees != 0) 
		{
			glm::vec2 o = origin + glm::vec2(position.x, -position.y) + glm::vec2(position.z, -position.w) / 2.f;

			p1 = rotateAroundPoint(p1, o, -rotationDegrees);
			p2 = rotateAroundPoint(p2, o, -rotationDegrees);
			p3 = rotateAroundPoint(p3, o, -rotationDegrees);
			p4 = rotateAroundPoint(p4, o, -rotationDegrees);
			p5 = rotateAroundPoint(p5, o, -rotationDegrees);
			p6 = rotateAroundPoint(p6, o, -rotationDegrees);
			p7 = rotateAroundPoint(p7, o, -rotationDegrees);
			p8 = rotateAroundPoint(p8, o, -rotationDegrees);
		}

		auto renderPoint = [&](glm::vec2 pos) 
		{
			renderRectangle({pos - glm::vec2(1,1),2,2}, Colors_Black);
		};

		renderPoint(p1);
		renderPoint(p2);
		renderPoint(p3);
		renderPoint(p4);
		renderPoint(p5);
		renderPoint(p6);
		renderPoint(p7);
		renderPoint(p8);

		//add a padding so the lines align properly.
		renderLine(p1, p2, color, width); //top line
		renderLine(p3, p4, color, width);
		renderLine(p5, p6, color, width); //bottom line
		renderLine(p7, p8, color, width);

	}

	void  Renderer2D::renderCircleOutline(const glm::vec2 position, const Color4f color, const float size, const float width, const unsigned int segments)
	{
	
		auto calcPos = [&](int p)
		{
			glm::vec2 circle = {size,0};

			float a = 3.1415926 * 2 * ((float)p / segments);

			float c = std::cos(a);
			float s = std::sin(a);

			circle = {c * circle.x - s * circle.y, s * circle.x + c * circle.y};

			return circle + position;
		};

		
		glm::vec2 lastPos = calcPos(1);
		renderLine(calcPos(0), lastPos, color, width);
		for (int i = 1; i < segments; i++)
		{

			glm::vec2 pos1 = lastPos;
			glm::vec2 pos2 = calcPos(i + 1);

			renderLine(pos1, pos2, color, width);

			lastPos = pos2;
		}

	}



	void Renderer2D::render9Patch(const Rect position, const int borderSize, const Color4f color, const glm::vec2 origin, const float rotation, const Texture texture, const Texture_Coords textureCoords, const Texture_Coords inner_texture_coords)
	{
		glm::vec4 colorData[4] = { color, color, color, color };

		//inner
		Rect innerPos = position;
		innerPos.x += borderSize;
		innerPos.y += borderSize;
		innerPos.z -= borderSize * 2;
		innerPos.w -= borderSize * 2;
		renderRectangle(innerPos, texture, colorData, Position2D{ 0, 0 }, 0, inner_texture_coords);

		//top
		Rect topPos = position;
		topPos.x += borderSize;
		topPos.z -= (float)borderSize * 2;
		topPos.w = (float)borderSize;
		glm::vec4 upperTexPos;
		upperTexPos.x = inner_texture_coords.x;
		upperTexPos.y = textureCoords.y;
		upperTexPos.z = inner_texture_coords.z;
		upperTexPos.w = inner_texture_coords.y;
		renderRectangle(topPos, texture, colorData, Position2D{ 0, 0 }, 0, upperTexPos);

		//bottom
		Rect bottom = position;
		bottom.x += (float)borderSize;
		bottom.y += (float)position.w - borderSize;
		bottom.z -= (float)borderSize * 2;
		bottom.w = (float)borderSize;
		glm::vec4 bottomTexPos;
		bottomTexPos.x = inner_texture_coords.x;
		bottomTexPos.y = inner_texture_coords.w;
		bottomTexPos.z = inner_texture_coords.z;
		bottomTexPos.w = textureCoords.w;
		renderRectangle(bottom, texture, colorData, Position2D{ 0, 0 }, 0, bottomTexPos);

		//left
		Rect left = position;
		left.y += borderSize;
		left.z = (float)borderSize;
		left.w -= (float)borderSize * 2;
		glm::vec4 leftTexPos;
		leftTexPos.x = textureCoords.x;
		leftTexPos.y = inner_texture_coords.y;
		leftTexPos.z = inner_texture_coords.x;
		leftTexPos.w = inner_texture_coords.w;
		renderRectangle(left, texture, colorData, Position2D{ 0, 0 }, 0, leftTexPos);

		//right
		Rect right = position;
		right.x += position.z - borderSize;
		right.y += borderSize;
		right.z = (float)borderSize;
		right.w -= (float)borderSize * 2;
		glm::vec4 rightTexPos;
		rightTexPos.x = inner_texture_coords.z;
		rightTexPos.y = inner_texture_coords.y;
		rightTexPos.z = textureCoords.z;
		rightTexPos.w = inner_texture_coords.w;
		renderRectangle(right, texture, colorData, Position2D{ 0, 0 }, 0, rightTexPos);

		//topleft
		Rect topleft = position;
		topleft.z = (float)borderSize;
		topleft.w = (float)borderSize;
		glm::vec4 topleftTexPos;
		topleftTexPos.x = textureCoords.x;
		topleftTexPos.y = textureCoords.y;
		topleftTexPos.z = inner_texture_coords.x;
		topleftTexPos.w = inner_texture_coords.y;
		renderRectangle(topleft, texture, colorData, Position2D{ 0, 0 }, 0, topleftTexPos);

		//topright
		Rect topright = position;
		topright.x += position.z - borderSize;
		topright.z = (float)borderSize;
		topright.w = (float)borderSize;
		glm::vec4 toprightTexPos;
		toprightTexPos.x = inner_texture_coords.z;
		toprightTexPos.y = textureCoords.y;
		toprightTexPos.z = textureCoords.z;
		toprightTexPos.w = inner_texture_coords.y;
		renderRectangle(topright, texture, colorData, Position2D{ 0, 0 }, 0, toprightTexPos);

		//bottomleft
		Rect bottomleft = position;
		bottomleft.y += position.w - borderSize;
		bottomleft.z = (float)borderSize;
		bottomleft.w = (float)borderSize;
		glm::vec4 bottomleftTexPos;
		bottomleftTexPos.x = textureCoords.x;
		bottomleftTexPos.y = inner_texture_coords.w;
		bottomleftTexPos.z = inner_texture_coords.x;
		bottomleftTexPos.w = textureCoords.w;
		renderRectangle(bottomleft, texture, colorData, Position2D{ 0, 0 }, 0, bottomleftTexPos);

		//bottomright
		Rect bottomright = position;
		bottomright.y += position.w - borderSize;
		bottomright.x += position.z - borderSize;
		bottomright.z = (float)borderSize;
		bottomright.w = (float)borderSize;
		glm::vec4 bottomrightTexPos;
		bottomrightTexPos.x = inner_texture_coords.z;
		bottomrightTexPos.y = inner_texture_coords.w;
		bottomrightTexPos.z = textureCoords.z;
		bottomrightTexPos.w = textureCoords.w;
		renderRectangle(bottomright, texture, colorData, Position2D{ 0, 0 }, 0, bottomrightTexPos);

	}

	void Renderer2D::render9Patch2(const Rect position, const Color4f color, const glm::vec2 origin, const float rotation, const Texture texture, const Texture_Coords textureCoords, const Texture_Coords inner_texture_coords)
	{
		glm::vec4 colorData[4] = { color, color, color, color };

		int w = 0;
		int h = 0;
		glBindTexture(GL_TEXTURE_2D, texture.id);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &w);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h);

		float textureSpaceW = textureCoords.z - textureCoords.x;
		float textureSpaceH = textureCoords.y - textureCoords.w;

		float topBorder = (textureCoords.y - inner_texture_coords.y) / textureSpaceH * position.w;
		float bottomBorder = (inner_texture_coords.w - textureCoords.w) / textureSpaceH * position.w;
		float leftBorder = (inner_texture_coords.x - textureCoords.x) / textureSpaceW * position.z;
		float rightBorder = (textureCoords.z - inner_texture_coords.z) / textureSpaceW * position.z;

		float newAspectRatio = position.z / position.w;
		
		if (newAspectRatio < 1.f) 
		{
			topBorder *= newAspectRatio;
			bottomBorder *= newAspectRatio;
		}
		else
		{
			leftBorder /= newAspectRatio;
			rightBorder /= newAspectRatio;
		}

	

		//topBorder = 50;
		//bottomBorder = -50;
		//leftBorder = 0;
		//rightBorder = 0;


		//inner
		Rect innerPos = position;
		innerPos.x += leftBorder;
		innerPos.y += topBorder;
		innerPos.z -= leftBorder + rightBorder;
		innerPos.w -= topBorder + bottomBorder;
		renderRectangle(innerPos, texture, colorData, Position2D{ 0, 0 }, 0, inner_texture_coords);

		//top
		Rect topPos = position;
		topPos.x += leftBorder;
		topPos.z -= leftBorder + rightBorder;
		topPos.w = topBorder;
		glm::vec4 upperTexPos;
		upperTexPos.x = inner_texture_coords.x;
		upperTexPos.y = textureCoords.y;
		upperTexPos.z = inner_texture_coords.z;
		upperTexPos.w = inner_texture_coords.y;
		renderRectangle(topPos, texture, colorData, Position2D{ 0, 0 }, 0, upperTexPos);

		//Rect topPos = position;
		//topPos.x += leftBorder;
		//topPos.w = topBorder;
		//topPos.z = topBorder;
		//float end = rightBorder;
		//float size = topBorder;
		//
		//while(1)
		//{
		//	if(topPos.x + size <= end)
		//	{
		//
		//		//draw
		//		renderRectangle(topPos, colorData, Position2D{ 0, 0 }, 0, texture, upperTexPos);
		//
		//		topPos += size;
		//	}else
		//	{
		//		float newW = end - topPos.x;
		//		if(newW>0)
		//		{
		//			topPos.z = newW;
		//			renderRectangle(topPos, colorData, Position2D{ 0, 0 }, 0, texture, upperTexPos);
		//		}
		//		break;
		//	}
		//
		//}


		//bottom
		Rect bottom = position;
		bottom.x += leftBorder;
		bottom.y += (float)position.w - bottomBorder;
		bottom.z -= leftBorder + rightBorder;
		bottom.w = bottomBorder;
		glm::vec4 bottomTexPos;
		bottomTexPos.x = inner_texture_coords.x;
		bottomTexPos.y = inner_texture_coords.w;
		bottomTexPos.z = inner_texture_coords.z;
		bottomTexPos.w = textureCoords.w;
		renderRectangle(bottom, texture, colorData, Position2D{ 0, 0 }, 0, bottomTexPos);

		//left
		Rect left = position;
		left.y += topBorder;
		left.z = leftBorder;
		left.w -= topBorder + bottomBorder;
		glm::vec4 leftTexPos;
		leftTexPos.x = textureCoords.x;
		leftTexPos.y = inner_texture_coords.y;
		leftTexPos.z = inner_texture_coords.x;
		leftTexPos.w = inner_texture_coords.w;
		renderRectangle(left, texture, colorData, Position2D{ 0, 0 }, 0, leftTexPos);

		//right
		Rect right = position;
		right.x += position.z - rightBorder;
		right.y += topBorder;
		right.z = rightBorder;
		right.w -= topBorder + bottomBorder;
		glm::vec4 rightTexPos;
		rightTexPos.x = inner_texture_coords.z;
		rightTexPos.y = inner_texture_coords.y;
		rightTexPos.z = textureCoords.z;
		rightTexPos.w = inner_texture_coords.w;
		renderRectangle(right, texture, colorData, Position2D{ 0, 0 }, 0, rightTexPos);

		//topleft
		Rect topleft = position;
		topleft.z = leftBorder;
		topleft.w = topBorder;
		glm::vec4 topleftTexPos;
		topleftTexPos.x = textureCoords.x;
		topleftTexPos.y = textureCoords.y;
		topleftTexPos.z = inner_texture_coords.x;
		topleftTexPos.w = inner_texture_coords.y;
		renderRectangle(topleft, texture, colorData, Position2D{ 0, 0 }, 0, topleftTexPos);
		//repair here?


		//topright
		Rect topright = position;
		topright.x += position.z - rightBorder;
		topright.z = rightBorder;
		topright.w = topBorder;
		glm::vec4 toprightTexPos;
		toprightTexPos.x = inner_texture_coords.z;
		toprightTexPos.y = textureCoords.y;
		toprightTexPos.z = textureCoords.z;
		toprightTexPos.w = inner_texture_coords.y;
		renderRectangle(topright, texture, colorData, Position2D{ 0, 0 }, 0, toprightTexPos);

		//bottomleft
		Rect bottomleft = position;
		bottomleft.y += position.w - bottomBorder;
		bottomleft.z = leftBorder;
		bottomleft.w = bottomBorder;
		glm::vec4 bottomleftTexPos;
		bottomleftTexPos.x = textureCoords.x;
		bottomleftTexPos.y = inner_texture_coords.w;
		bottomleftTexPos.z = inner_texture_coords.x;
		bottomleftTexPos.w = textureCoords.w;
		renderRectangle(bottomleft, texture, colorData, Position2D{ 0, 0 }, 0, bottomleftTexPos);

		//bottomright
		Rect bottomright = position;
		bottomright.y += position.w - bottomBorder;
		bottomright.x += position.z - rightBorder;
		bottomright.z = rightBorder;
		bottomright.w = bottomBorder;
		glm::vec4 bottomrightTexPos;
		bottomrightTexPos.x = inner_texture_coords.z;
		bottomrightTexPos.y = inner_texture_coords.w;
		bottomrightTexPos.z = textureCoords.z;
		bottomrightTexPos.w = textureCoords.w;
		renderRectangle(bottomright, texture, colorData, Position2D{ 0, 0 }, 0, bottomrightTexPos);

	}

	void Renderer2D::create(GLuint fbo, size_t quadCount)
	{
		if (!hasInitialized)
		{
			errorFunc("Library not initialized. Have you forgotten to call gl2d::init() ?", userDefinedData);
		}

		defaultFBO = fbo;

		clearDrawData();
		spritePositions.reserve(quadCount * 6);
		spriteColors.reserve(quadCount * 6);
		texturePositions.reserve(quadCount * 6);
		spriteTextures.reserve(quadCount);

		this->resetCameraAndShader();

		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		glGenBuffers(Renderer2DBufferType::bufferSize, buffers);

		glBindBuffer(GL_ARRAY_BUFFER, buffers[Renderer2DBufferType::quadPositions]);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glBindBuffer(GL_ARRAY_BUFFER, buffers[Renderer2DBufferType::quadColors]);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glBindBuffer(GL_ARRAY_BUFFER, buffers[Renderer2DBufferType::texturePositions]);
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glBindVertexArray(0);
	}

	void Renderer2D::cleanup()
	{
		glDeleteVertexArrays(1, &vao);
		glDeleteBuffers(Renderer2DBufferType::bufferSize, buffers);
	}

	void Renderer2D::pushShader(ShaderProgram s)
	{
		shaderPushPop.push_back(currentShader);
		currentShader = s;
	}

	void Renderer2D::popShader()
	{
		if (shaderPushPop.empty())
		{
			errorFunc("Pop on an empty stack on popShader", userDefinedData);
		}
		else
		{
			currentShader = shaderPushPop.back();
			shaderPushPop.pop_back();
		}
	}

	void Renderer2D::pushCamera(Camera c)
	{
		cameraPushPop.push_back(currentCamera);
		currentCamera = c;
	}

	void Renderer2D::popCamera()
	{
		if (cameraPushPop.empty())
		{
			errorFunc("Pop on an empty stack on popCamera", userDefinedData);
		}
		else
		{
			currentCamera = cameraPushPop.back();
			cameraPushPop.pop_back();
		}
	}

	glm::vec4 Renderer2D::getViewRect()
	{
		auto rect = glm::vec4{0, 0, windowW, windowH};

		glm::mat3 mat = 
		{1.f, 0, currentCamera.position.x ,
		 0, 1.f, currentCamera.position.y,
		 0, 0, 1.f};
		mat = glm::transpose(mat);

		glm::vec3 pos1 = {rect.x, rect.y, 1.f};
		glm::vec3 pos2 = {rect.z + rect.x, rect.w + rect.y, 1.f};

		pos1 = mat * pos1;
		pos2 = mat * pos2;
		
		glm::vec2 point((pos1.x + pos2.x) / 2.f, (pos1.y + pos2.y) / 2.f);

		pos1 = glm::vec3(scaleAroundPoint(pos1, point, 1.f/currentCamera.zoom), 1.f);
		pos2 = glm::vec3(scaleAroundPoint(pos2, point, 1.f/currentCamera.zoom), 1.f);

		rect = {pos1.x, pos1.y, pos2.x - pos1.x, pos2.y - pos1.y};

		return rect;
	}

	glm::vec4 Renderer2D::toScreen(const glm::vec4& transform)
	{
		//We need to flip texture_transforms.y
		const float transformsY = transform.y * -1;

		glm::vec2 v1 = { transform.x,				  transformsY };
		glm::vec2 v2 = { transform.x,				  transformsY - transform.w };
		glm::vec2 v3 = { transform.x + transform.z, transformsY - transform.w };
		glm::vec2 v4 = { transform.x + transform.z, transformsY };

		//Apply camera transformations
		v1.x -= currentCamera.position.x;
		v1.y += currentCamera.position.y;
		v2.x -= currentCamera.position.x;
		v2.y += currentCamera.position.y;
		v3.x -= currentCamera.position.x;
		v3.y += currentCamera.position.y;
		v4.x -= currentCamera.position.x;
		v4.y += currentCamera.position.y;

		//Apply camera zoom
		//if(renderer->currentCamera.zoom != 1)
		{

			glm::vec2 cameraCenter;
			cameraCenter.x = windowW / 2.0f;
			cameraCenter.y = -windowH / 2.0f;

			v1 = scaleAroundPoint(v1, cameraCenter, currentCamera.zoom);
			v3 = scaleAroundPoint(v3, cameraCenter, currentCamera.zoom);
		}

		v1.x = internal::positionToScreenCoordsX(v1.x, (float)windowW);
		v3.x = internal::positionToScreenCoordsX(v3.x, (float)windowW);
		v1.y = internal::positionToScreenCoordsY(v1.y, (float)windowH);
		v3.y = internal::positionToScreenCoordsY(v3.y, (float)windowH);

		return glm::vec4(v1.x, v1.y, v3.x, v3.y);
	}

	glm::vec2 Renderer2D::getTextSize(const char *text, const Font font,
		const float size, const float spacing, const float line_space)
	{
		if (font.texture.id == 0)
		{
			errorFunc("Missing font", userDefinedData);
			return {};
		}

		glm::vec2 position = {};

		const int text_length = (int)strlen(text);
		Rect rectangle = {};
		rectangle.x = position.x;
		float linePositionY = position.y;

		//This is the y position we render at because it advances when we encounter newlines
		float maxPos = 0;
		float maxPosY = 0;
		float bonusY = 0;

		for (int i = 0; i < text_length; i++)
		{
			if (text[i] == '\n')
			{
				rectangle.x = position.x;
				linePositionY += (font.max_height + line_space) * size;
				bonusY += (font.max_height + line_space) * size;
				maxPosY = 0;
			}
			else if (text[i] == '\t')
			{
				const stbtt_aligned_quad quad = internal::fontGetGlyphQuad
				(font, '_');
				auto x = quad.x1 - quad.x0;

				rectangle.x += x * size * 3 + spacing * size;
			}
			else if (text[i] == ' ')
			{
				const stbtt_aligned_quad quad = internal::fontGetGlyphQuad
				(font, '_');
				auto x = quad.x1 - quad.x0;

				rectangle.x += x * size + spacing * size;
			}
			else if (text[i] >= ' ' && text[i] <= '~')
			{
				const stbtt_aligned_quad quad = internal::fontGetGlyphQuad
				(font, text[i]);

				rectangle.z = quad.x1 - quad.x0;
				rectangle.w = quad.y1 - quad.y0;

				rectangle.z *= size;
				rectangle.w *= size;

				rectangle.y = linePositionY + quad.y0 * size;

				rectangle.x += rectangle.z + spacing * size;

				maxPosY = std::max(maxPosY, rectangle.y);
				maxPos = std::max(maxPos, rectangle.x);
			}
		}

		maxPos = std::max(maxPos, rectangle.x);
		maxPosY = std::max(maxPosY, rectangle.y);

		float paddX = maxPos;

		float paddY = maxPosY;

		paddY += font.max_height * size + bonusY;

		return glm::vec2{paddX, paddY};

	}

	float Renderer2D::determineTextRescaleFitSmaller(const std::string &str,
		gl2d::Font &f, glm::vec4 transform, float maxSize)
	{
		auto s = getTextSize(str.c_str(), f, maxSize);

		float ratioX = transform.z / s.x;
		float ratioY = transform.w / s.y;


		if (ratioX > 1 && ratioY > 1)
		{
			return maxSize;
		}
		else
		{
			if (ratioX < ratioY)
			{
				return maxSize*ratioX;
			}
			else
			{
				return maxSize * ratioY;
			}
		}
	}


	float Renderer2D::determineTextRescaleFitBigger(const std::string &str,
		gl2d::Font &f, glm::vec4 transform, float minSize)
	{
		auto s = getTextSize(str.c_str(), f, minSize);

		float ratioX = transform.z / s.x;
		float ratioY = transform.w / s.y;


		if (ratioX > 1 && ratioY > 1)
		{
			if (ratioX > ratioY)
			{
				return minSize * ratioY;
			}
			else
			{
				return minSize * ratioX;
			}
		}
		else
		{
			
		}

		return minSize;
	
	}

	float Renderer2D::determineTextRescaleFit(const std::string &str,
		gl2d::Font &f, glm::vec4 transform)
	{
		float ret = 1;

		auto s = getTextSize(str.c_str(), f, ret);

		float ratioX = transform.z / s.x;
		float ratioY = transform.w / s.y;


		if (ratioX > 1 && ratioY > 1)
		{
			if (ratioX > ratioY)
			{
				return ret * ratioY;
			}
			else
			{
				return ret * ratioX;
			}
		}
		else
		{
			if (ratioX < ratioY)
			{
				return ret * ratioX;
			}
			else
			{
				return ret * ratioY;
			}
		}

		return ret;
	}

	int  Renderer2D::wrap(const std::string &in, gl2d::Font &f,
		float baseSize, float maxDimension, std::string *outRez)
	{
		if (outRez)
		{
			*outRez = "";
			outRez->reserve(in.size() + 10);
		}

		std::string word = "";
		std::string currentLine = "";
		currentLine.reserve(in.size() + 10);

		bool wrap = 0;
		bool newLine = 1;
		int newLineCounter = 0;

		for (int i = 0; i < in.size(); i++)
		{
			word.push_back(in[i]);
			currentLine.push_back(in[i]);

			if (in[i] == ' ')
			{
				if (wrap)
				{
					if (outRez)
					{
						outRez->push_back('\n'); currentLine = "";
					}
					newLineCounter++;

				}

				if (outRez)
				{
					*outRez += word;
				}
				word = "";
				wrap = 0;
				newLine = false;
			}
			else if (in[i] == '\n')
			{
				if (wrap)
				{
					if (outRez)
					{
						outRez->push_back('\n');
					}
					newLineCounter++;
				}

				currentLine = "";

				if (outRez)
				{
					*outRez += word;
				}
				word = "";
				wrap = 0;
				newLine = true;
			}
			else
			{
				//let's check, only if needed
				if (!wrap && !newLine)
				{
					float size = baseSize;
					auto textSize = getTextSize(currentLine.c_str(), f, size);

					if (textSize.x >= maxDimension && !newLine)
					{
						//wrap last word
						wrap = 1;
					}
				};
			}

		}

		{
			if (wrap) { if (outRez)outRez->push_back('\n'); newLineCounter++; }

			if (outRez)
			{
				*outRez += word;
			}
		}

		return newLineCounter + 1;
	}

	void Renderer2D::renderText(glm::vec2 position, const char *text, const Font font,
		const Color4f color, const float size, const float spacing, const float line_space, bool showInCenter,
		const Color4f ShadowColor
		, const Color4f LightColor
	)
	{
		if (font.texture.id == 0)
		{
			errorFunc("Missing font", userDefinedData);
			return;
		}

		const int text_length = (int)strlen(text);
		Rect rectangle;
		rectangle.x = position.x;
		float linePositionY = position.y;

		if (showInCenter)
		{
			//This is the y position we render at because it advances when we encounter newlines

			float maxPos = 0;
			float maxPosY = 0;

			for (int i = 0; i < text_length; i++)
			{
				if (text[i] == '\n')
				{
					rectangle.x = position.x;
					linePositionY += (font.max_height + line_space) * size;
				}
				else if (text[i] == '\t')
				{
					const stbtt_aligned_quad quad = internal::fontGetGlyphQuad
					(font, '_');
					auto x = quad.x1 - quad.x0;

					rectangle.x += x * size * 3 + spacing * size;
				}
				else if (text[i] == ' ')
				{
					const stbtt_aligned_quad quad = internal::fontGetGlyphQuad
					(font, '_');
					auto x = quad.x1 - quad.x0;

					rectangle.x += x * size + spacing * size;
				}
				else if (text[i] >= ' ' && text[i] <= '~')
				{
					const stbtt_aligned_quad quad = internal::fontGetGlyphQuad
					(font, text[i]);

					rectangle.z = quad.x1 - quad.x0;
					rectangle.w = quad.y1 - quad.y0;

					rectangle.z *= size;
					rectangle.w *= size;

					rectangle.y = linePositionY + quad.y0 * size;


					rectangle.x += rectangle.z + spacing * size;
					maxPos = std::max(maxPos, rectangle.x);
					maxPosY = std::max(maxPosY, rectangle.y);
				}
			}

			float padd = maxPos - position.x;
			padd /= 2;
			position.x -= padd;

			float paddY = maxPosY - position.y;
			position.y -= paddY;
		}

		rectangle = {};
		rectangle.x = position.x;

		//This is the y position we render at because it advances when we encounter newlines
		linePositionY = position.y;

		for (int i = 0; i < text_length; i++)
		{
			if (text[i] == '\n')
			{
				rectangle.x = position.x;
				linePositionY += (font.max_height + line_space) * size;
			}
			else if (text[i] == '\t')
			{
				const stbtt_aligned_quad quad = internal::fontGetGlyphQuad
				(font, '_');
				auto x = quad.x1 - quad.x0;

				rectangle.x += x * size * 3 + spacing * size;
			}
			else if (text[i] == ' ')
			{
				const stbtt_aligned_quad quad = internal::fontGetGlyphQuad
				(font, '_');
				auto x = quad.x1 - quad.x0;
				rectangle.x += x * size + spacing * size;
			}
			else if (text[i] >= ' ' && text[i] <= '~')
			{

				const stbtt_aligned_quad quad = internal::fontGetGlyphQuad
				(font, text[i]);

				rectangle.z = quad.x1 - quad.x0;
				rectangle.w = quad.y1 - quad.y0;

				rectangle.z *= size;
				rectangle.w *= size;

				//rectangle.y = linePositionY - rectangle.w;
				rectangle.y = linePositionY + quad.y0 * size;

				glm::vec4 colorData[4] = {color, color, color, color};

				if (ShadowColor.w)
				{
					glm::vec2 pos = {-5, 3};
					pos *= size;
					renderRectangle({rectangle.x + pos.x, rectangle.y + pos.y,  rectangle.z, rectangle.w},
						font.texture, ShadowColor, glm::vec2{0, 0}, 0,
						glm::vec4{quad.s0, quad.t0, quad.s1, quad.t1});

				}

				renderRectangle(rectangle, font.texture, colorData, glm::vec2{0, 0}, 0,
					glm::vec4{quad.s0, quad.t0, quad.s1, quad.t1});

				if (LightColor.w)
				{
					glm::vec2 pos = {-2, 1};
					pos *= size;
					renderRectangle({rectangle.x + pos.x, rectangle.y + pos.y,  rectangle.z, rectangle.w},
						font.texture,
						LightColor, glm::vec2{0, 0}, 0,
						glm::vec4{quad.s0, quad.t0, quad.s1, quad.t1});

				}


				rectangle.x += rectangle.z + spacing * size;
			}
		}
	}

	void Renderer2D::renderTextWrapped(const std::string &text,
		gl2d::Font f, glm::vec4 textPos, glm::vec4 color, float baseSize,
		float spacing, float lineSpacing,
		bool showInCenter, glm::vec4 shadowColor, glm::vec4 lightColor)
	{
		std::string newText;
		wrap(text, f, baseSize, textPos.z, &newText);
		renderText(textPos,
			newText.c_str(), f, color, baseSize, spacing, lineSpacing, showInCenter,
			shadowColor, lightColor);
	}

	glm::vec2 Renderer2D::getTextSizeWrapped(const std::string &text,
		gl2d::Font f, float maxTextLenght, float baseSize, float spacing, float lineSpacing)
	{
		std::string newText;
		wrap(text, f, baseSize, maxTextLenght, &newText);
		auto rez = getTextSize(
			newText.c_str(), f, baseSize, spacing, lineSpacing);

		return rez;
	}

	void Renderer2D::clearScreen(const Color4f color)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, defaultFBO);
	
		#if GL2D_USE_OPENGL_130
			GLfloat oldColor[4];
			glGetFloatv(GL_COLOR_CLEAR_VALUE, oldColor);

			glClearColor(color.r, color.g, color.b, color.a);
			glClear(GL_COLOR_BUFFER_BIT);
			glClearColor(oldColor[0], oldColor[1], oldColor[2], oldColor[3]);
		#else
			glClearBufferfv(GL_COLOR, 0, &color[0]);
		#endif

	}

	void Renderer2D::setShaderProgram(const ShaderProgram shader)
	{
		currentShader = shader;
	}

	void Renderer2D::setCamera(const Camera camera)
	{
		currentCamera = camera;
	}

	void Renderer2D::resetCameraAndShader()
	{
		currentCamera = defaultCamera;
		currentShader = defaultShader;
	}

#pragma endregion

	glm::ivec2 Texture::GetSize()
	{
		glm::ivec2 s;
		glBindTexture(GL_TEXTURE_2D, id);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &s.x);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &s.y);
		return s;
	}

	void Texture::createFromBuffer(const char* image_data, const int width, const int height
		,bool pixelated, bool useMipMaps)
	{
		GLuint id = 0;

		glActiveTexture(GL_TEXTURE0);

		glGenTextures(1, &id);
		glBindTexture(GL_TEXTURE_2D, id);

		if (pixelated)
		{
			if (useMipMaps)
			{
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
			}
			else 
			{
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			}
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		}
		else 
		{
			if (useMipMaps)
			{
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			}
			else
			{
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			}
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
		glGenerateMipmap(GL_TEXTURE_2D);


		this->id = id;
	}

	void Texture::create1PxSquare(const char* b)
	{
		if (b == nullptr)
		{
			const unsigned char buff[] =
			{
				0xff,
				0xff,
				0xff,
				0xff
			};

			createFromBuffer((char*)buff, 1, 1);
		}
		else
		{
			createFromBuffer(b, 1, 1);
		}

	}

	void Texture::createFromFileData(const unsigned char* image_file_data, const size_t image_file_size
		,bool pixelated, bool useMipMaps)
	{
		stbi_set_flip_vertically_on_load(true);

		int width = 0;
		int height = 0;
		int channels = 0;

		const unsigned char* decodedImage = stbi_load_from_memory(image_file_data, (int)image_file_size, &width, &height, &channels, 4);

		createFromBuffer((const char*)decodedImage, width, height, pixelated, useMipMaps);

		STBI_FREE(decodedImage);
	}

	void Texture::createFromFileDataWithPixelPadding(const unsigned char* image_file_data, const size_t image_file_size, int blockSize,
		bool pixelated, bool useMipMaps)
	{
		stbi_set_flip_vertically_on_load(true);

		int width = 0;
		int height = 0;
		int channels = 0;

		const unsigned char* decodedImage = stbi_load_from_memory(image_file_data, (int)image_file_size, &width, &height, &channels, 4);

		int newW = width + ((width * 2) / blockSize);
		int newH = height + ((height * 2) / blockSize);

		auto getOld = [decodedImage, width](int x, int y, int c)->const unsigned char
		{
			return decodedImage[4 * (x + (y * width)) + c];
		};


		unsigned char* newData = new unsigned char[newW * newH * 4]{};

		auto getNew = [newData, newW](int x, int y, int c)
		{
			return &newData[4 * (x + (y * newW)) + c];
		};

		int newDataCursor = 0;
		int dataCursor = 0;

		//first copy data
		for (int y = 0; y < newH; y++)
		{
			int yNo = 0;
			if ((y == 0 || y == newH - 1
				|| ((y) % (blockSize + 2)) == 0 ||
				((y + 1) % (blockSize + 2)) == 0
				))
			{
				yNo = 1;
			}

			for (int x = 0; x < newW; x++)
			{
				if (
					yNo ||

					((
						x == 0 || x == newW - 1
						|| (x % (blockSize + 2)) == 0 ||
						((x + 1) % (blockSize + 2)) == 0
						)
						)

					)
				{
					newData[newDataCursor++] = 0;
					newData[newDataCursor++] = 0;
					newData[newDataCursor++] = 0;
					newData[newDataCursor++] = 0;
				}
				else
				{
					newData[newDataCursor++] = decodedImage[dataCursor++];
					newData[newDataCursor++] = decodedImage[dataCursor++];
					newData[newDataCursor++] = decodedImage[dataCursor++];
					newData[newDataCursor++] = decodedImage[dataCursor++];
				}

			}

		}

		//then add margins


		for (int x = 1; x < newW - 1; x++)
		{
			//copy on left
			if (x == 1 ||
				(x % (blockSize + 2)) == 1
				)
			{
				for (int y = 0; y < newH; y++)
				{
					*getNew(x - 1, y, 0) = *getNew(x, y, 0);
					*getNew(x - 1, y, 1) = *getNew(x, y, 1);
					*getNew(x - 1, y, 2) = *getNew(x, y, 2);
					*getNew(x - 1, y, 3) = *getNew(x, y, 3);
				}

			}
			else //copy on rigght
				if (x == newW - 2 ||
					(x % (blockSize + 2)) == blockSize
					)
				{
					for (int y = 0; y < newH; y++)
					{
						*getNew(x + 1, y, 0) = *getNew(x, y, 0);
						*getNew(x + 1, y, 1) = *getNew(x, y, 1);
						*getNew(x + 1, y, 2) = *getNew(x, y, 2);
						*getNew(x + 1, y, 3) = *getNew(x, y, 3);
					}
				}
		}

		for (int y = 1; y < newH - 1; y++)
		{
			if (y == 1 ||
				(y % (blockSize + 2)) == 1
				)
			{
				for (int x = 0; x < newW; x++)
				{
					*getNew(x, y - 1, 0) = *getNew(x, y, 0);
					*getNew(x, y - 1, 1) = *getNew(x, y, 1);
					*getNew(x, y - 1, 2) = *getNew(x, y, 2);
					*getNew(x, y - 1, 3) = *getNew(x, y, 3);
				}
			}
			else
				if (y == newH - 2 ||
					(y % (blockSize + 2)) == blockSize
					)
				{
					for (int x = 0; x < newW; x++)
					{
						*getNew(x, y + 1, 0) = *getNew(x, y, 0);
						*getNew(x, y + 1, 1) = *getNew(x, y, 1);
						*getNew(x, y + 1, 2) = *getNew(x, y, 2);
						*getNew(x, y + 1, 3) = *getNew(x, y, 3);
					}
				}

		}

		createFromBuffer((const char*)newData, newW, newH, pixelated, useMipMaps);

		STBI_FREE(decodedImage);
		delete[] newData;
	}

	void Texture::loadFromFile(const char* fileName, bool pixelated, bool useMipMaps)
	{
		std::ifstream file(fileName, std::ios::binary);

		if (!file.is_open())
		{
			char c[300] = { 0 };
			strcat(c, "error openning: ");
			strcat(c + strlen(c), fileName);
			errorFunc(c, userDefinedData);
			return;
		}

		int fileSize = 0;
		file.seekg(0, std::ios::end);
		fileSize = (int)file.tellg();
		file.seekg(0, std::ios::beg);
		unsigned char* fileData = new unsigned char[fileSize];
		file.read((char*)fileData, fileSize);
		file.close();

		createFromFileData(fileData, fileSize, pixelated, useMipMaps);

		delete[] fileData;

	}

	void Texture::loadFromFileWithPixelPadding(const char* fileName, int blockSize,
		bool pixelated, bool useMipMaps)
	{
		std::ifstream file(fileName, std::ios::binary);

		if (!file.is_open())
		{
			char c[300] = { 0 };
			strcat(c, "error openning: ");
			strcat(c + strlen(c), fileName);
			errorFunc(c, userDefinedData);
			return;
		}

		int fileSize = 0;
		file.seekg(0, std::ios::end);
		fileSize = (int)file.tellg();
		file.seekg(0, std::ios::beg);
		unsigned char* fileData = new unsigned char[fileSize];
		file.read((char*)fileData, fileSize);
		file.close();

		createFromFileDataWithPixelPadding(fileData, fileSize, blockSize, pixelated, useMipMaps);

		delete[] fileData;

	}

	size_t Texture::getMemorySize(int mipLevel, glm::ivec2 *outSize)
	{
		glBindTexture(GL_TEXTURE_2D, id);

		glm::ivec2 stub = {};

		if (!outSize)
		{
			outSize = &stub;
		}

		glGetTexLevelParameteriv(GL_TEXTURE_2D, mipLevel, GL_TEXTURE_WIDTH, &outSize->x);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, mipLevel, GL_TEXTURE_HEIGHT, &outSize->y);
		
		glBindTexture(GL_TEXTURE_2D, 0);

		return outSize->x * outSize->y * 4;
	}

	void Texture::readTextureData(void *buffer, int mipLevel)
	{
		glBindTexture(GL_TEXTURE_2D, id);
		glGetTexImage(GL_TEXTURE_2D, mipLevel, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
	}

	std::vector<unsigned char> Texture::readTextureData(int mipLevel, glm::ivec2 *outSize)
	{
		glBindTexture(GL_TEXTURE_2D, id);

		glm::ivec2 stub = {};

		if (!outSize)
		{
			outSize = &stub;
		}

		glGetTexLevelParameteriv(GL_TEXTURE_2D, mipLevel, GL_TEXTURE_WIDTH, &outSize->x);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, mipLevel, GL_TEXTURE_HEIGHT, &outSize->y);

		std::vector<unsigned char> data;
		data.resize(outSize->x * outSize->y * 4);
		glGetTexImage(GL_TEXTURE_2D, mipLevel, GL_RGBA, GL_UNSIGNED_BYTE, data.data());

		glBindTexture(GL_TEXTURE_2D, 0);

		return data;
	}

	void Texture::bind(const unsigned int sample)
	{
		glActiveTexture(GL_TEXTURE0 + sample);
		glBindTexture(GL_TEXTURE_2D, id);
	}

	void Texture::unbind()
	{
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	void Texture::cleanup()
	{
		glDeleteTextures(1, &id);
		*this = {};
	}

	//glm::mat3 Camera::getMatrix()
	//{
	//	glm::mat3 m;
	//	m = { zoom, 0, position.x ,
	//		 0, zoom, position.y,
	//		0, 0, 1,
	//	};
	//	m = glm::transpose(m);
	//	return m; //todo not tested, add rotation
	//}

	void Camera::follow(glm::vec2 pos, float speed, float min, float max, float w, float h)
	{
		pos.x -= w / 2.f;
		pos.y -= h / 2.f;

		glm::vec2 delta = pos - position;
		bool signX = delta.x >= 0;
		bool signY = delta.y >= 0;

		float len = glm::length(delta);

		delta = glm::normalize(delta);

		if (len < min * 2)
		{
			speed /= 4.f;
		}
		else if (len < min * 4)
		{
			speed /= 2.f;
		}

		if (len > min)
		{
			if (len > max)
			{
				len = max;
				position = pos - (max * delta);
				//osition += delta * speed;
			}
			else
			{
				position += delta * speed;


			}

			glm::vec2 delta2 = pos - position;
			bool signX2 = delta.x >= 0;
			bool signY2 = delta.y >= 0;
			if (signX2 != signX || signY2 != signY || glm::length(delta2) > len)
			{
				//position = pos;
			}
		}
	}

	glm::vec2 internal::convertPoint(const Camera &camera, const glm::vec2& p, float windowW, float windowH)
	{
		glm::vec2 r = p;


		//Apply camera transformations
		r.x += camera.position.x;
		r.y += camera.position.y;

		{
			glm::vec2 cameraCenter = { camera.position.x + windowW / 2, -camera.position.y - windowH / 2 };

			r = rotateAroundPoint(r,
				cameraCenter,
				camera.rotation);
		}

		{
			glm::vec2 cameraCenter = { camera.position.x + windowW / 2, camera.position.y + windowH / 2 };

			r = scaleAroundPoint(r,
				cameraCenter,
				1.f / camera.zoom);
		}

		//if (this->rotation != 0)
		//{
		//	glm::vec2 cameraCenter;
		//
		//	cameraCenter.x = windowW / 2.0f;
		//	cameraCenter.y = windowH / 2.0f;
		//
		//	r = rotateAroundPoint(r, cameraCenter, this->rotation);
		//
		//}

		//{
		//	glm::vec2 cameraCenter;
		//	cameraCenter.x = windowW / 2.0f;
		//	cameraCenter.y = -windowH / 2.0f;
		//
		//	r = scaleAroundPoint(r, cameraCenter, this->zoom);
		//
		//}

		return r;
	}

	void FrameBuffer::create(unsigned int w, unsigned int h)
	{
		glGenFramebuffers(1, &fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);

		glGenTextures(1, &texture.id);
		glBindTexture(GL_TEXTURE_2D, texture.id);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture.id, 0);

		//glDrawBuffer(GL_COLOR_ATTACHMENT0); //todo why is this commented out ?

		//glGenTextures(1, &depthtTexture);
		//glBindTexture(GL_TEXTURE_2D, depthtTexture);

		//glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, w, h, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);

		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthtTexture, 0);

		glBindTexture(GL_TEXTURE_2D, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

	}

	void FrameBuffer::resize(unsigned int w, unsigned int h)
	{
		glBindTexture(GL_TEXTURE_2D, texture.id);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

		//glBindTexture(GL_TEXTURE_2D, depthtTexture);
		//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	}

	void FrameBuffer::cleanup()
	{
		if (fbo)
		{
			glDeleteFramebuffers(1, &fbo);
			fbo = 0;
		}

		if (texture.id)
		{
			glDeleteTextures(1, &texture.id);
			texture = {};
		}

		//glDeleteTextures(1, &depthtTexture);
		//depthtTexture = 0;
	}

	void FrameBuffer::clear()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		//glClearColor(1, 1, 1, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//glClearColor(0, 0, 0, 0);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}


	glm::vec4 computeTextureAtlas(int xCount, int yCount, int x, int y, bool flip)
	{
		float xSize = 1.f / xCount;
		float ySize = 1.f / yCount;

		if (flip)
		{
			return { (x + 1) * xSize, 1 - (y * ySize), (x)*xSize, 1.f - ((y + 1) * ySize) };
		}
		else
		{
			return { x * xSize, 1 - (y * ySize), (x + 1) * xSize, 1.f - ((y + 1) * ySize) };
		}

	}

	glm::vec4 computeTextureAtlasWithPadding(int mapXsize, int mapYsize,
		int xCount, int yCount, int x, int y, bool flip)
	{
		float xSize = 1.f / xCount;
		float ySize = 1.f / yCount;

		float Xpadding = 1.f / mapXsize;
		float Ypadding = 1.f / mapYsize;

		glm::vec4 noFlip = { x * xSize + Xpadding, 1 - (y * ySize) - Ypadding, (x + 1) * xSize - Xpadding, 1.f - ((y + 1) * ySize) + Ypadding };

		if (flip)
		{
			glm::vec4 flip = { noFlip.z, noFlip.y, noFlip.x, noFlip.w };

			return flip;
		}
		else
		{
			return noFlip;
		}
	}

	


}

#ifdef _MSC_VER
#pragma warning( pop )
#endif