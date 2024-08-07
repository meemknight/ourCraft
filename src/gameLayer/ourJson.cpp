#include <ourJson.h>
#include <safeSave/include/safeSave.h>
#include <platform/platformTools.h>
#include <iostream>

bool KeyValuePair::loadElementsFromFile(const char *fileName)
{

	std::vector<char> data;
	if (sfs::readEntireFile(data, fileName) == sfs::noError)
	{
		data.push_back(0);
		return loadElementsFromData(data.data());
	}
	else
	{
		return 0;
	}
}


bool KeyValuePair::loadElementsFromData(const char *data)
{

	*this = {};

	enum
	{
		TokenNone,
		TokenString,
		TokenNumber,
		TokenSymbol,
	};

	struct Token
	{
		int type = 0;
		std::string s = "";
		char symbol = 0;
		double number = 0;

		bool operator==(Token &other)
		{
			return type == other.type &&
				s == other.s &&
				symbol == other.symbol &&
				number == other.number;
		}
	};

	std::vector<Token> tokens;

#pragma region tokenize
	{
		int lastPos = 0;
		for (int i = 0; data[i] != '\0';)
		{
			auto checkSymbol = [&]() -> bool
			{
				if (data[i] == ';' || data[i] == ':' || data[i] == '{' || data[i] == '}' || data[i] == ',')
				{
					tokens.push_back({Token{TokenSymbol, "", data[i], 0}});
					i++;
					return true;
				}
				return 0;
			};

			auto checkNumber = [&]() -> bool
			{
				if (std::isdigit(data[i]) || (data[i] == '-' && std::isdigit(data[i + 1])))
				{
					Token t;
					t.type = TokenNumber;
					t.s += data[i];
					i++;

					for (; data[i] != '\0'; i++)
					{
						if (std::isdigit(data[i]) || data[i] == '.')
						{
							t.s += data[i];
						}
						else
						{
							break;
						}
					}
					t.number = std::stod(t.s); //todo replace with something that reports errors

					if (!t.s.empty()) { tokens.push_back(t); }

					return true;
				}

				return 0;
			};

			auto consumeWhiteSpaces = [&]()
			{
				for (; data[i] != '\0'; i++)
				{
					if (data[i] != '\n' && data[i] != '\v' && data[i] != '\t' && data[i] != ' ' && data[i] != '\r')
					{
						break;
					}
				}
			};

			auto addString = [&]()
			{
				Token t;
				t.type = TokenString;

				for (; data[i] != '\0'; i++)
				{
					if (isalnum(data[i]))
					{
						t.s += data[i];
					}
					else
					{
						break;
					}
				}

				if (!t.s.empty()) { tokens.push_back(t); }
			};

			consumeWhiteSpaces();
			if (data[i] == '\0') { break; }

			if (checkSymbol())continue;
			if (checkNumber())continue;
			addString();

			if (lastPos == i)
			{
				return 0;
			}
			else
			{
				lastPos = i;
			}
		}
	}
#pragma endregion


	for (int i = 0; i < tokens.size();)
	{

		auto isEof = [&]() { return i >= tokens.size(); };

		auto isString = [&]() -> bool
		{
			if (isEof()) { return false; }
			return tokens[i].type == TokenString;
		};

		auto consumeString = [&]()->std::string
		{
			if (tokens[i].type == TokenString)
			{
				i++;
				return tokens[i - 1].s;
			}

			permaAssert(0);
			return "";
		};

		auto consume = [&](Token t) -> bool
		{
			if (i >= tokens.size()) { return false; }

			if (tokens[i] == t)
			{
				i++;
				return true;
			}
			else
			{
				return false;
			}
		};

		auto isNumber = [&]()
		{
			if (isEof()) { return false; }
			return (tokens[i].type == TokenNumber);
		};

		auto consumeNumber = [&]() -> double
		{
			if (tokens[i].type == TokenNumber)
			{
				i++;
				return tokens[i - 1].number;
			};

			permaAssert(0);
			return 0;
		};

		auto isColon = [&]() -> bool
		{
			if (isEof()) { return false; }

			if (tokens[i].type == TokenSymbol)
			{
				return tokens[i].symbol = ':';
			};
			return 0;
		};

		auto isSemiColon = [&]() -> bool
		{
			if (isEof()) { return false; }

			if (tokens[i].type == TokenSymbol)
			{
				return tokens[i].symbol = ';';
			};
			return 0;
		};

		auto consumeEntry = [&]() -> bool
		{
			Entry entry;
			std::string entryName;

			if (isString())
			{
				entryName = consumeString();
				
				if (isColon())
				{
					i++;

					if (isString())
					{
						entry.type = Entry::stringType;
						entry.strValue = consumeString();
					}
					else if (isNumber())
					{
						entry.type = Entry::numberType;
						entry.numberValue = consumeNumber();
					}
					else
					{
						return 0;
					}

					if (!isSemiColon()) { return 0; }
					i++;

					elements[entryName] = entry;

					return 1;
				}
				else
				{
					return 0;
				}
			}
			else
			{
				return 0;
			}


		};

		while (!isEof())
		{
			bool rez = consumeEntry();

			if (!rez) { return 0; }
		}

		return 1;

	}




}

void KeyValuePair::printAll()
{

	for (auto &e : elements)
	{

		std::cout << e.first << ": ";

		if (e.second.type == Entry::stringType)
		{
			std::cout << "String: " << e.second.strValue;
		}
		else if (e.second.type == Entry::numberType)
		{
			std::cout << "Number: " << e.second.numberValue;
		}
		else if (e.second.type == Entry::boolType)
		{
			std::cout << "Bool: " << (bool)e.second.numberValue;
		}else{}
		
		std::cout << "\n";
	}

}

std::vector<char> KeyValuePair::formatIntoFileData()
{
	std::vector<char> data;

	data.reserve(elements.size() * 20);

	auto pushString = [&](const std::string &s)
	{
		for (const auto &c : s)
		{
			data.push_back(c);
		}
	};

	auto pushChar = [&](char c)
	{
		data.push_back(c);
	};

	for (auto &e : elements)
	{
		pushString(e.first);
		pushChar(':');
		pushChar(' ');

		if (e.second.type == Entry::stringType)
		{
			pushString(e.second.strValue);
		}
		else if (e.second.type == Entry::numberType)
		{
			pushString(std::to_string(e.second.numberValue));
		}
		else if (e.second.type == Entry::boolType)
		{
			pushString(std::to_string((bool)e.second.numberValue));
		}
		else {}

		pushChar(';');
		pushChar('\n');
	}

	return data;
}

void KeyValuePair::writeIntoFile(const char *fileName)
{
	
	sfs::writeEntireFile(formatIntoFileData(), fileName);

}
