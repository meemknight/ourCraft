#include <errorReporting.h>
#include <fstream>
#include <iostream>

void createErrorFile()
{
	std::fstream f(RESOURCES_PATH "../errorLogs.txt");
	f.close();
}



void reportError(const char *message)
{
	std::fstream f(RESOURCES_PATH "../errorLogs.txt", std::ios::app);
	f << message << "\n";
	f.close();
	std::cout << message << "\n";
}