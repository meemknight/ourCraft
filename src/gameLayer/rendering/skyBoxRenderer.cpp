#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "rendering/skyBoxRenderer.h"
#include <stb_image/stb_image.h>
#include <platformTools.h>
#include <iostream>
#include <fstream>


float skyboxVertices[] = {
	// positions          
	-1.0f,  1.0f, -1.0f,
	-1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,

	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,

	-1.0f,  1.0f, -1.0f,
	 1.0f,  1.0f, -1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	 1.0f, -1.0f,  1.0f
};


static glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
static glm::mat4 captureViews[] =
{
   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
};

void SkyBoxLoaderAndDrawer::createGpuData()
{


	atmosphericScatteringShader.shader.loadShaderProgramFromFile(RESOURCES_PATH  "shaders/skyBox/hdrToCubeMap.vert",
		RESOURCES_PATH  "shaders/skyBox/atmosphericScattering.frag");
	atmosphericScatteringShader.u_lightPos = getUniform(atmosphericScatteringShader.shader.id, "u_lightPos");
	atmosphericScatteringShader.u_color1 = getUniform(atmosphericScatteringShader.shader.id, "u_color1");
	atmosphericScatteringShader.u_color2 = getUniform(atmosphericScatteringShader.shader.id, "u_color2");
	atmosphericScatteringShader.u_groundColor = getUniform(atmosphericScatteringShader.shader.id, "u_groundColor");
	atmosphericScatteringShader.u_g = getUniform(atmosphericScatteringShader.shader.id, "u_g");
	atmosphericScatteringShader.u_useGround = getUniform(atmosphericScatteringShader.shader.id, "u_useGround");
	atmosphericScatteringShader.u_viewProjection = getUniform(atmosphericScatteringShader.shader.id, "u_viewProjection");
	atmosphericScatteringShader.u_noSun = getUniform(atmosphericScatteringShader.shader.id, "u_noSun");
	

	normalSkyBox.shader.loadShaderProgramFromFile(RESOURCES_PATH "shaders/skyBox/skyBox.vert", RESOURCES_PATH "shaders/skyBox/skyBox.frag");
	normalSkyBox.modelViewUniformLocation = getUniform(normalSkyBox.shader.id, "u_viewProjection");
	normalSkyBox.u_sunPos = getUniform(normalSkyBox.shader.id, "u_sunPos");
	normalSkyBox.u_blend = getUniform(normalSkyBox.shader.id, "u_blend");
	normalSkyBox.u_rotation1 = getUniform(normalSkyBox.shader.id, "u_rotation1");
	normalSkyBox.u_rotation2 = getUniform(normalSkyBox.shader.id, "u_rotation2");


	hdrtoCubeMap.shader.loadShaderProgramFromFile(RESOURCES_PATH  "shaders/skyBox/hdrToCubeMap.vert", RESOURCES_PATH  "shaders/skyBox/hdrToCubeMap.frag");
	hdrtoCubeMap.u_equirectangularMap = getUniform(hdrtoCubeMap.shader.id, "u_equirectangularMap");
	hdrtoCubeMap.modelViewUniformLocation = getUniform(hdrtoCubeMap.shader.id, "u_viewProjection");

	convolute.shader.loadShaderProgramFromFile(RESOURCES_PATH "shaders/skyBox/hdrToCubeMap.vert",
		RESOURCES_PATH "shaders/skyBox/convolute.frag");
	convolute.u_environmentMap = getUniform(convolute.shader.id, "u_environmentMap");
	convolute.modelViewUniformLocation = getUniform(convolute.shader.id, "u_viewProjection");
	convolute.u_sampleQuality = getUniform(convolute.shader.id, "u_sampleQuality");

	preFilterSpecular.shader.loadShaderProgramFromFile(RESOURCES_PATH "shaders/skyBox/hdrToCubeMap.vert",
		RESOURCES_PATH "shaders/skyBox/preFilterSpecular.frag");
	preFilterSpecular.modelViewUniformLocation = getUniform(preFilterSpecular.shader.id, "u_viewProjection");
	preFilterSpecular.u_environmentMap = getUniform(preFilterSpecular.shader.id, "u_environmentMap");
	preFilterSpecular.u_roughness = getUniform(preFilterSpecular.shader.id, "u_roughness");
	preFilterSpecular.u_sampleCount = getUniform(preFilterSpecular.shader.id, "u_sampleCount");
	

	glGenVertexArrays(1, &vertexArray);
	glBindVertexArray(vertexArray);

	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

	glBindVertexArray(0);


	glGenFramebuffers(1, &captureFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO); //also allocate gpu resources
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//render out the sky textures
	//createSkyTextures();


	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void SkyBoxLoaderAndDrawer::createSkyTextures()
{

	daySky.clearTextures();
	atmosphericScattering({0, -1, 0}, glm::vec3{9, 47, 116} / 255.f,
		glm::vec3{33,38,44} / 255.f, {1,1,1}, false, 0.4, daySky, true);

	twilightSky.clearTextures();
	atmosphericScattering(glm::normalize(glm::vec3{0, -0.1, 1}), glm::vec3{22, 49, 111} / 255.f,
		glm::vec3{35,22,36} / 255.f, {1,1,1}, false, 0.995, twilightSky, false);

	nightSky.clearTextures();
	atmosphericScattering({0, -1, 0}, glm::vec3{9, 15, 23} / 255.f * 0.4f,
		glm::vec3{4,8,10} / 255.f * 0.4f, {1,1,1}, false, 0.850, nightSky, true);

}

void SkyBoxLoaderAndDrawer::atmosphericScattering(glm::vec3 sunPos,
	glm::vec3 color1, glm::vec3 color2,
	glm::vec3 groundColor, bool useGroundColor, float g,
	SkyBox &skyBox, bool noSun)
{
	skyBox = {};
	constexpr int skyBoxSize = 128;

	//render into the cubemap
	{
		GLuint captureFBO;
		glGenFramebuffers(1, &captureFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);

		glGenTextures(1, &skyBox.texture);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skyBox.texture);
		for (unsigned int i = 0; i < 6; ++i)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_R11F_G11F_B10F,
				skyBoxSize, skyBoxSize, 0, GL_RGB, GL_FLOAT, nullptr);
		}
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		//rendering
		{

			atmosphericScatteringShader.shader.bind();
			glUniform3fv(atmosphericScatteringShader.u_lightPos, 1, &sunPos[0]);
			glUniform1f(atmosphericScatteringShader.u_g, g);
			glUniform3fv(atmosphericScatteringShader.u_color1, 1, &color1[0]);
			glUniform3fv(atmosphericScatteringShader.u_color2, 1, &color2[0]);
			glUniform3fv(atmosphericScatteringShader.u_groundColor, 1, &groundColor[0]);
			glUniform1i(atmosphericScatteringShader.u_useGround, (int)useGroundColor);
			glUniform1i(atmosphericScatteringShader.u_noSun, (int)noSun);

			glViewport(0, 0, skyBoxSize, skyBoxSize);

			glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
			glBindVertexArray(vertexArray);

			for (unsigned int i = 0; i < 6; ++i)
			{
				glm::mat4 viewProjMat = captureProjection * captureViews[i];
				glUniformMatrix4fv(atmosphericScatteringShader.u_viewProjection, 1, GL_FALSE, &viewProjMat[0][0]);
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
					GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, skyBox.texture, 0);
				glClear(GL_COLOR_BUFFER_BIT);

				glDrawArrays(GL_TRIANGLES, 0, 6 * 6); // renders a 1x1 cube
			}

			glBindVertexArray(0);

		}

		glDeleteFramebuffers(1, &captureFBO);

	}

	createConvolutedAndPrefilteredTextureData(skyBox, 0.02, 64u);

}


void SkyBoxLoaderAndDrawer::loadAllTextures(std::string path)
{

	createSkyTextures();
	//if (!daySky.texture)
	//{
	//	loadTexture((path + "skybox.png").c_str(), daySky);
	//
	//}

	
	//if (!nightSky.texture)
	//{
	//	loadTexture((path + "nightsky.png").c_str(), nightSky);
	//}

	//if (!twilightSky.texture)
	//{
	//	loadTexture((path + "twilightsky.png").c_str(), twilightSky);
	//}

	if (!sunTexture.id)
	{
		sunTexture.loadFromFile((path + "sun.png").c_str(), false, false);
	}

	if (!moonTexture.id)
	{
		moonTexture.loadFromFile((path + "moon.png").c_str(), false, false);
	}
}

void SkyBoxLoaderAndDrawer::clearOnlyTextures()
{
	//daySky.clearTextures(); daySky = {};
	//nightSky.clearTextures(); nightSky = {};
	//twilightSky.clearTextures(); twilightSky = {};
	sunTexture.cleanup(); sunTexture = {};
	moonTexture.cleanup(); moonTexture = {};
}

void SkyBoxLoaderAndDrawer::loadTexture(const char *name, SkyBox &skyBox, int format)
{
	skyBox = {};

	int width = 0, height = 0, nrChannels = 0;
	unsigned char *data = 0;

	stbi_set_flip_vertically_on_load(false);

	std::ifstream file(name, std::ios::binary);
	if (!file.is_open())
	{
		std::cout << "Error, Coult not open file: " << name << "\n";
		return;
	}

	GLint size = 0;
	file.seekg(0, file.end);
	size = file.tellg();
	file.seekg(0, file.beg);
	char *fileContent = new char[size + 1]{};
	file.read(fileContent, size);
	file.close();

	data = stbi_load_from_memory((unsigned char *)fileContent, size, &width, &height, &nrChannels, 3);
	delete[] fileContent;


	if (!data)
	{
		std::cout << "Err parsing: " << name << "\n";
		return;
	}

	glGenTextures(1, &skyBox.texture);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skyBox.texture);

	//right
	//left
	//top
	//bottom
	//front//?
	//back //?

	auto getPixel = [&](int x, int y, unsigned char *data)
	{
		return data + 3 * (x + y * width);
	};

	glm::ivec2 paddings[6];
	glm::ivec2 immageRatio = {};
	int flipX[6] = {};
	int flipY[6] = {};

	if (format == 0)
	{
		immageRatio = {4, 3};
		glm::ivec2 paddingscopy[6] =
		{
			{ (width / 4) * 2, (height / 3) * 1, },
			{ (width / 4) * 0, (height / 3) * 1, },
			{ (width / 4) * 1, (height / 3) * 0, },
			{ (width / 4) * 1, (height / 3) * 2, },
			{ (width / 4) * 1, (height / 3) * 1, },
			{ (width / 4) * 3, (height / 3) * 1, },
		};

		memcpy(paddings, paddingscopy, sizeof(paddings));


	}
	else if (format == 1)
	{
		immageRatio = {3, 4};
		glm::ivec2 paddingscopy[6] =
		{
			{ (width / 3) * 2, (height / 4) * 1, },
			{ (width / 3) * 0, (height / 4) * 1, },
			{ (width / 3) * 1, (height / 4) * 0, },
			{ (width / 3) * 1, (height / 4) * 2, },
			{ (width / 3) * 1, (height / 4) * 1, },
			{ (width / 3) * 1, (height / 4) * 3, },
		};

		memcpy(paddings, paddingscopy, sizeof(paddings));
		flipX[5] = 1;
		flipY[5] = 1;

	}
	else if (format == 2)
	{
		immageRatio = {4, 3};
		glm::ivec2 paddingscopy[6] =
		{
			{ (width / 4) * 3, (height / 3) * 1, },
			{ (width / 4) * 1, (height / 3) * 1, },
			{ (width / 4) * 2, (height / 3) * 0, },
			{ (width / 4) * 2, (height / 3) * 2, },
			{ (width / 4) * 2, (height / 3) * 1, },
			{ (width / 4) * 0, (height / 3) * 1, },

		};

		memcpy(paddings, paddingscopy, sizeof(paddings));

	}
	else
	{
		permaAssertComment(0, "invalid format for texture");
	}


	if (data)
	{
		for (unsigned int i = 0; i < 6; i++)
		{
			unsigned char *extractedData = new unsigned char[3 *
				(width / immageRatio.x) * (height / immageRatio.y)];

			int index = 0;

			int paddingX = paddings[i].x;
			int paddingY = paddings[i].y;

		#pragma region flip

			int jBeg = 0;
			int jAdvance = 1;
			if (flipY[i])
			{
				jBeg = height / immageRatio.y - 1;
				jAdvance = -1;
			}

			int xBeg = 0;
			int xAdvance = 1;
			if (flipX[i])
			{
				xBeg = width / immageRatio.x - 1;
				xAdvance = -1;
			}
		#pragma endregion


			for (int j = jBeg; j < height / immageRatio.y && j >= 0; j += jAdvance)
				for (int i = xBeg; i < width / immageRatio.x && i >= 0; i += xAdvance)
				{
					extractedData[index] = *getPixel(i + paddingX, j + paddingY, data);
					extractedData[index + 1] = *(getPixel(i + paddingX, j + paddingY, data) + 1);
					extractedData[index + 2] = *(getPixel(i + paddingX, j + paddingY, data) + 2);
					//extractedData[index] = 100;
					//extractedData[index + 1] = 100;
					//extractedData[index + 2] = 100;
					index += 3;
				}

			glTexImage2D(
				GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0, GL_SRGB, width / immageRatio.x, height / immageRatio.y, 0,
				GL_RGB, GL_UNSIGNED_BYTE, extractedData
			);



			delete[] extractedData;
		}

		stbi_image_free(data);

	}



	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);


	createConvolutedAndPrefilteredTextureData(skyBox);

}

void SkyBoxLoaderAndDrawer::loadHDRtexture(const char *name, SkyBox &skyBox, GLuint frameBuffer)
{
	skyBox = {};

	int width = 0, height = 0, nrChannels = 0;
	float *data = 0;

	stbi_set_flip_vertically_on_load(true);


	std::ifstream file(name, std::ios::binary);
	if (!file.is_open())
	{
		std::cout << "Error, Coult not open file: " << name << "\n";
		glDeleteTextures(1, &skyBox.texture);
		return;
	}

	GLint size = 0;
	file.seekg(0, file.end);
	size = file.tellg();
	file.seekg(0, file.beg);
	char *fileContent = new char[size + 1]{};
	file.read(fileContent, size);
	file.close();

	data = stbi_loadf_from_memory((unsigned char *)fileContent, size, &width, &height, &nrChannels, 3);

	if (!data) 
	{ 
		std::cout << "Err parsing: " << name << "\n";
		delete[] fileContent;
		return; 
	}


	GLuint hdrTexture;

	glGenTextures(1, &hdrTexture);
	glBindTexture(GL_TEXTURE_2D, hdrTexture);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	stbi_image_free(data);
	delete[] fileContent;

	//render into the cubemap
	//https://learnopengl.com/PBR/IBL/Diffuse-irradiance
	{
		GLuint captureFBO; //todo cache this
		glGenFramebuffers(1, &captureFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);


		glGenTextures(1, &skyBox.texture);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skyBox.texture);
		for (unsigned int i = 0; i < 6; ++i)
		{
			// note that we store each face with 16 bit floating point values
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F,
				512, 512, 0, GL_RGB, GL_FLOAT, nullptr);
		}
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		//rendering
		{

			hdrtoCubeMap.shader.bind();
			glUniform1i(hdrtoCubeMap.u_equirectangularMap, 0);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, hdrTexture);

			glViewport(0, 0, 512, 512);

			glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
			glBindVertexArray(vertexArray);

			for (unsigned int i = 0; i < 6; ++i)
			{
				glm::mat4 viewProjMat = captureProjection * captureViews[i];
				glUniformMatrix4fv(hdrtoCubeMap.modelViewUniformLocation, 1, GL_FALSE, &viewProjMat[0][0]);
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
					GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, skyBox.texture, 0);
				glClear(GL_COLOR_BUFFER_BIT);

				glDrawArrays(GL_TRIANGLES, 0, 6 * 6); // renders a 1x1 cube
			}

			glBindVertexArray(0);
			glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);

		}

		glDeleteFramebuffers(1, &captureFBO);

	}

	glDeleteTextures(1, &hdrTexture);

	createConvolutedAndPrefilteredTextureData(skyBox, frameBuffer);
}

void SkyBoxLoaderAndDrawer::createConvolutedAndPrefilteredTextureData(SkyBox &skyBox,
	float sampleQuality, unsigned int specularSamples)
{
	glBindTexture(GL_TEXTURE_CUBE_MAP, skyBox.texture);
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);


	GLint viewPort[4] = {};
	glGetIntegerv(GL_VIEWPORT, viewPort);

	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

#pragma region convoluted texturelayout(binding = 4) uniform sampler2D u_roughness;


	glGenTextures(1, &skyBox.convolutedTexture);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skyBox.convolutedTexture);
	for (unsigned int i = 0; i < 6; ++i)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 32, 32, 0,
			GL_RGB, GL_FLOAT, nullptr);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);

	convolute.shader.bind();
	glUniform1i(convolute.u_environmentMap, 0);
	glUniform1f(convolute.u_sampleQuality, sampleQuality);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skyBox.texture);
	glViewport(0, 0, 32, 32);

	glBindVertexArray(vertexArray);

	for (unsigned int i = 0; i < 6; ++i)
	{

		glm::mat4 viewProjMat = captureProjection * captureViews[i];
		glUniformMatrix4fv(convolute.modelViewUniformLocation, 1, GL_FALSE, &viewProjMat[0][0]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, skyBox.convolutedTexture, 0);

		glClear(GL_COLOR_BUFFER_BIT);

		glDrawArrays(GL_TRIANGLES, 0, 6 * 6); // renders a 1x1 cube, todo refactor to draw only a face lol

	}
#pragma endregion


#pragma region prefiltered map

	constexpr int maxMipMap = 5;

	glGenTextures(1, &skyBox.preFilteredMap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skyBox.preFilteredMap);
	for (unsigned int i = 0; i < 6; ++i)
	{
		//todo mabe be able to tweak rezolution
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 128, 128, 0, GL_RGB, GL_FLOAT, nullptr);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, maxMipMap);
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	preFilterSpecular.shader.bind();
	glUniform1i(preFilterSpecular.u_environmentMap, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skyBox.texture);


	for (int mip = 0; mip < maxMipMap; mip++)
	{
		unsigned int mipWidth = 128 * std::pow(0.5, mip);
		unsigned int mipHeight = 128 * std::pow(0.5, mip);
		glViewport(0, 0, mipWidth, mipHeight);

		float roughness = (float)mip / (float)(maxMipMap - 1);
		roughness *= roughness;
		glUniform1f(preFilterSpecular.u_roughness, roughness);
		glUniform1ui(preFilterSpecular.u_sampleCount, specularSamples);

		for (int i = 0; i < 6; i++)
		{
			glm::mat4 viewProjMat = captureProjection * captureViews[i];
			glUniformMatrix4fv(preFilterSpecular.modelViewUniformLocation, 1, GL_FALSE, &viewProjMat[0][0]);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
				GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, skyBox.preFilteredMap, mip);
			glClear(GL_COLOR_BUFFER_BIT);

			glDrawArrays(GL_TRIANGLES, 0, 6 * 6); // renders a 1x1 cube
		}
	}


#pragma endregion
	//reset stuff

	glDisable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	glBindVertexArray(0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(viewPort[0], viewPort[1], viewPort[2], viewPort[3]);

	//texture = convolutedTexture; //visualize convolutex texture
	//texture = preFilteredMap;

	glBindTexture(GL_TEXTURE_CUBE_MAP, skyBox.texture);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	
}

void SkyBoxLoaderAndDrawer::drawBefore(const glm::mat4 &viewProjMat,
	glm::vec3 sunPos, float timeOfDay)
{

	//calculate time of day stuff
	if (timeOfDay > 1.f)
	{
		timeOfDay -= (int)timeOfDay;
	}

	float rotation1 = 0;
	float rotation2 = 0;
	{
		float rot = clock() / 80'000.f; //todo
		if (timeOfDay <= 0.25)
		{
			rotation1 = 3.141592f/2.f;
			rotation2 = rot;
		}
		else if (timeOfDay <= 0.50)
		{
			rotation1 = rot;
			rotation2 = -3.141592f / 2.f;
		}
		else if (timeOfDay <= 0.75)
		{
			rotation1 = -3.141592f / 2.f;
			rotation2 = rot;
		}
		else if (timeOfDay <= 1.f)
		{
			rotation1 = rot;
			rotation2 = 3.141592f / 2.f;
		}
	}

	GLint firstTexture = 0;
	GLint secondTexture = 0;
	float mix = 0;

	float bias = 2;

	if (timeOfDay <= 0.25)
	{
		firstTexture = twilightSky.texture;
		secondTexture = daySky.texture;
		mix = std::powf(timeOfDay / 0.25f, 1.f/bias);
	}
	else if (timeOfDay <= 0.50)
	{
		firstTexture = daySky.texture;
		secondTexture = twilightSky.texture;
		mix = std::powf((timeOfDay -0.25f) / 0.25f, bias);
	}
	else if (timeOfDay <= 0.75)
	{
		firstTexture = twilightSky.texture;
		secondTexture = nightSky.texture;
		mix = std::powf((timeOfDay - 0.50f) / 0.25f, 1.f/bias);
	}
	else if (timeOfDay <= 1.f)
	{
		firstTexture = nightSky.texture;
		secondTexture = twilightSky.texture;
		mix = std::powf((timeOfDay - 0.75f) / 0.25f, bias);
	}


	glBindVertexArray(vertexArray);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, firstTexture);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_CUBE_MAP, secondTexture);

	normalSkyBox.shader.bind();
	
	glUniform1f(normalSkyBox.u_blend, mix);
	glUniform1f(normalSkyBox.u_rotation1, rotation1);
	glUniform1f(normalSkyBox.u_rotation2, rotation2);

	glUniformMatrix4fv(normalSkyBox.modelViewUniformLocation, 1, GL_FALSE, &viewProjMat[0][0]);
	glUniform3fv(normalSkyBox.u_sunPos, 1, glm::value_ptr(sunPos));


	glDisable(GL_DEPTH_TEST);
	glDrawArrays(GL_TRIANGLES, 0, 6 * 6);
	glEnable(GL_DEPTH_TEST);

	glBindVertexArray(0);
}

void SkyBoxLoaderAndDrawer::clearOnlyGPUdata()
{
	atmosphericScatteringShader.shader.clear();
	normalSkyBox.shader.clear();
	hdrtoCubeMap.shader.clear();
	convolute.shader.clear();
	preFilterSpecular.shader.clear();
	glDeleteVertexArrays(1, &vertexArray);
	glDeleteBuffers(1, &vertexBuffer);
	glDeleteFramebuffers(1, &captureFBO);
}

void SkyBox::clearTextures()
{
	if (texture)
	{
		glDeleteTextures(1, &texture);
	}

	if (convolutedTexture)
	{
		glDeleteTextures(1, &convolutedTexture);
	}

	if (preFilteredMap)
	{
		glDeleteTextures(1, &preFilteredMap);
	}

	texture = 0;
	convolutedTexture = 0;
	preFilteredMap = 0;
}


float sunBufferData[] = 
{
	//pos    uv
	1,1,    1,1,
	-1, 1,  0, 1,
	-1, -1, 0, 0,
	1, -1,  1, 0
};


void SunRenderer::create()
{
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);

	glBufferStorage(GL_ARRAY_BUFFER, sizeof(sunBufferData), sunBufferData, 0);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, 0);


	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (void*)(sizeof(float) * 2));


	shader.loadShaderProgramFromFile(RESOURCES_PATH "shaders/skyBox/sun.vert", 
		RESOURCES_PATH "shaders/skyBox/sun.frag");
	
	u_modelViewProjectionMatrix = getUniform(shader.id, "u_modelViewProjectionMatrix");

	glBindVertexArray(0);
}


//orientates something facing at (0,0,-1) to be at direction
glm::mat4 createTBNMatrix(const glm::vec3 &direction, const glm::vec3 &up = glm::vec3(0.0f, 1.0f, 0.0f))
{
	// Normalize the direction vector
	glm::vec3 forward = -glm::normalize(direction);

	// Calculate the right vector
	glm::vec3 right = glm::normalize(glm::cross(up, forward));

	// Recalculate the up vector as it's perpendicular to both forward and right vectors
	glm::vec3 newUp = glm::cross(forward, right);

	// Create a 3x3 rotation matrix using the right, up, and forward vectors
	glm::mat3 rotationMatrix = glm::mat3(right, newUp, forward);

	// Convert to a 4x4 matrix
	return glm::mat4(rotationMatrix);
}

void SunRenderer::render(Camera camera, 
	glm::vec3 sunPos, gl2d::Texture sunTexture)
{
	glBindVertexArray(vao);
	shader.bind();

	sunTexture.bind(0);

	auto finalMatrix = camera.getProjectionMatrix() * camera.getViewMatrix() * createTBNMatrix(sunPos);
	glUniformMatrix4fv(u_modelViewProjectionMatrix, 1, GL_FALSE, &finalMatrix[0][0]);

	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

void SunRenderer::clear()
{

	glDeleteBuffers(1, &vertexBuffer);
	glDeleteVertexArrays(1, &vao);

	shader.clear();
	*this = {};
}

glm::vec3 calculateSunPosition(float dayTime)
{
	if (dayTime > 1.f) {dayTime -= (int)dayTime;}

	//0   -> sunrise
	//0.25 -> noon
	//0.75 -> noon
	//1    -> sunrise

	float angle = dayTime * 2 * 3.1415926;

	glm::vec4 finalAngle = {1,0,0,1};
	finalAngle = glm::rotate(angle, glm::vec3{0,0,1}) * finalAngle;

	return finalAngle;
}
