#pragma once
#include <glad/glad.h>

struct Shader
{
	GLuint id = 0;

	bool loadShaderProgramFromFile(const char* vertexShader, const char* fragmentShader);
	bool loadShaderProgramFromFile(const char* vertexShader,
		const char* geometryShader, const char* fragmentShader);

	void bind();

	void clear();

	GLint getUniform(const char* name);
	GLint getUniformSubroutine(GLenum shaderType, const char* name);
	GLuint getUniformBlock(const char* name);
	GLuint getUniformSubroutineIndex(GLenum shaderType, const char* name);
	GLuint getStorageBlockIndex(const char* name);
};

GLint getUniform(GLuint id, const char* name);
GLint getUniformSubroutine(GLuint id, GLenum shaderType, const char* name);
GLuint getUniformBlock(GLuint id, const char* name);
GLuint getUniformSubroutineIndex(GLuint id, GLenum shaderType, const char* name);
GLuint getStorageBlockIndex(GLuint id, const char* name);



