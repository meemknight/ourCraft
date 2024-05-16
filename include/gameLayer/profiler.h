#pragma once
#include <profilerLib.h>
#include <unordered_map>
#include <platformTools.h>
#include <deque>
#include <string>

struct CharEquals
{
	bool operator()(const char * &__x, const char *&__y) const
	{
		return strcmp(__x, __y) == 0;
	}
};



struct Profiler
{
	bool pause = 0;

	struct SavedData
	{
		float dataMs[10];
		float dataMsReal[10];
	};


	PL::Profiler mainProfiler;

	void startFrame();

	void endFrame();

	void startSubProfile(char *c);
	void endSubProfile(char *c);

	void setSubProfileManually(char *c, PL::ProfileRezults rezults);

	std::unordered_map<std::string, PL::Profiler> subProfiles;

	void displayPlot(const char *mainPlotName);

	std::deque<SavedData> history;

};