#include "shader.h"
#include <iostream>
#include <fstream>

GLint createShaderFromData(const char* data, GLenum shaderType)
{
	GLuint shaderId = glCreateShader(shaderType);
	glShaderSource(shaderId, 1, &data, nullptr);
	glCompileShader(shaderId);

	GLint rezult = 0;
	glGetShaderiv(shaderId, GL_COMPILE_STATUS, &rezult);

	if (!rezult)
	{
		char* message = 0;
		int   l = 0;

		glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &l);

		if (l)
		{
			message = new char[l];

			glGetShaderInfoLog(shaderId, l, &l, message);

			message[l - 1] = 0;

			std::cout << data << ":\n" << message << "\n";

			delete[] message;

		}
		else
		{
			std::cout << data << ":\n" << "unknown error" << "\n";
		}

		glDeleteShader(shaderId);

		shaderId = 0;
		return shaderId;
	}

	return shaderId;

}

GLint createShaderFromFile(const char* source, GLenum shaderType)
{
	std::ifstream file;
	file.open(source);

	if (!file.is_open())
	{
		std::cout << "Error openning file: " << source << "\n";
		return 0;
	}

	GLint size = 0;
	file.seekg(0, file.end);
	size = file.tellg();
	file.seekg(0, file.beg);

	char* fileContent = new char[size + 1]{};

	file.read(fileContent, size);


	file.close();

	auto rez = createShaderFromData(fileContent, shaderType);

	delete[] fileContent;

	return rez;

}

bool Shader::loadShaderProgramFromFile(const char* vertexShader, const char* fragmentShader)
{

	auto vertexId = createShaderFromFile(vertexShader, GL_VERTEX_SHADER);
	auto fragmentId = createShaderFromFile(fragmentShader, GL_FRAGMENT_SHADER);


	if (vertexId == 0 || fragmentId == 0)
	{
		return 0;
	}

	id = glCreateProgram();

	glAttachShader(id, vertexId);
	glAttachShader(id, fragmentId);

	glLinkProgram(id);

	glDeleteShader(vertexId);
	glDeleteShader(fragmentId);

	GLint info = 0;
	glGetProgramiv(id, GL_LINK_STATUS, &info);

	if (info != GL_TRUE)
	{
		char* message = 0;
		int   l = 0;

		glGetProgramiv(id, GL_INFO_LOG_LENGTH, &l);

		message = new char[l];

		glGetProgramInfoLog(id, l, &l, message);

		std::cout << "Link error: " << message << "\n";

		delete[] message;

		glDeleteProgram(id);
		id = 0;
		return 0;
	}

	glValidateProgram(id);

	return true;
}

bool Shader::loadShaderProgramFromFile(const char* vertexShader, const char* geometryShader, const char* fragmentShader)
{

	auto vertexId = createShaderFromFile(vertexShader, GL_VERTEX_SHADER);
	auto geometryId = createShaderFromFile(geometryShader, GL_GEOMETRY_SHADER);
	auto fragmentId = createShaderFromFile(fragmentShader, GL_FRAGMENT_SHADER);

	if (vertexId == 0 || fragmentId == 0 || geometryId == 0)
	{
		return 0;
	}

	id = glCreateProgram();

	glAttachShader(id, vertexId);
	glAttachShader(id, geometryId);
	glAttachShader(id, fragmentId);

	glLinkProgram(id);

	glDeleteShader(vertexId);
	glDeleteShader(geometryId);
	glDeleteShader(fragmentId);

	GLint info = 0;
	glGetProgramiv(id, GL_LINK_STATUS, &info);

	if (info != GL_TRUE)
	{
		char* message = 0;
		int   l = 0;

		glGetProgramiv(id, GL_INFO_LOG_LENGTH, &l);

		message = new char[l];

		glGetProgramInfoLog(id, l, &l, message);

		std::cout << "Link error: " << message << "\n";

		delete[] message;

		glDeleteProgram(id);
		id = 0;
		return 0;
	}

	glValidateProgram(id);

	return true;
}

void Shader::bind()
{
	glUseProgram(id);
}

void Shader::clear()
{
	glDeleteProgram(id);
	id = 0;
}

GLint Shader::getUniform(const char* name)
{
	return ::getUniform(id, name);
}

GLint Shader::getUniformSubroutine(GLenum shaderType, const char* name)
{
	return ::getUniformSubroutine(id, shaderType, name);
}

GLuint Shader::getUniformBlock(const char* name)
{
	return ::getUniformBlock(id, name);
}

GLuint Shader::getUniformSubroutineIndex(GLenum shaderType, const char* name)
{
	return ::getUniformSubroutineIndex(id, shaderType, name);
}

GLuint Shader::getStorageBlockIndex(const char* name)
{
	return ::getStorageBlockIndex(id, name);
}

GLint getUniformSubroutine(GLuint id, GLenum shaderType, const char* name)
{
	GLint uniform = glGetSubroutineUniformLocation(id, shaderType, name);
	if (uniform == -1)
	{
		std::cout << "uniform subroutine error " << name << "\n";
	}
	return uniform;
};

GLint getUniform(GLuint id, const char* name)
{
	GLint uniform = glGetUniformLocation(id, name);
	if (uniform == -1)
	{
		std::cout << "uniform error " << name << "\n";
	}
	return uniform;
};

GLuint getUniformBlock(GLuint id, const char* name)
{
	GLuint uniform = glGetUniformBlockIndex(id, name);
	if (uniform == GL_INVALID_INDEX)
	{
		std::cout << "uniform block error " << name << "\n";
	}
	return uniform;
};

GLuint getUniformSubroutineIndex(GLuint id, GLenum shaderType, const char* name)
{
	GLuint uniform = glGetSubroutineIndex(id, shaderType, name);
	if (uniform == GL_INVALID_INDEX)
	{
		std::cout << "uniform subroutine index error " << name << "\n";
	}
	return uniform;
};

GLuint getStorageBlockIndex(GLuint id, const char* name)
{
	GLuint uniform = glGetProgramResourceIndex(id, GL_SHADER_STORAGE_BLOCK, name);
	if (uniform == GL_INVALID_INDEX)
	{
		std::cout << "storage block index error " << name << "\n";
	}
	return uniform;
};