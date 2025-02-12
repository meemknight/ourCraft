#pragma once
#include <vector>
#include <string>

constexpr int Token_None = 0;
constexpr int Token_String = 1;
constexpr int Token_Number = 2;
constexpr int Token_Symbol = 3;
constexpr int Token_Comment = 4;
constexpr int Token_Directive = 5; //for # stuff

struct Token
{

	unsigned int start = 0;
	unsigned int end = 0;
	unsigned int type = 0;

};

struct UniformEntry
{
	std::string name;
	int type = 0;
	bool asColor = 0;
};


std::vector<Token> tokenizeGLSL(const char *text);

bool hasVersion(std::vector<Token> &tokens, const char *text, int *posInTextEnd);

bool hasMainFunction(std::vector<Token> &tokens, const char *text);

bool hasMainColorOutput(std::vector<Token> &tokens, const char *text, std::string *name = 0, int *type = 0);

bool hasUniform(std::vector<Token> &tokens, const char *text, const char* name, const char* type);

std::vector<UniformEntry> getUniforms(std::vector<Token> &tokens, const char *text);