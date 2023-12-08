#pragma once
#include <profilerLib.h>
#include <imgui.h>
#include <unordered_map>
#include <platformTools.h>
#include <deque>

struct CharEquals
{
	bool operator()(const char * &__x, const char *&__y) const
	{
		return strcmp(__x, __y) == 0;
	}
};



struct Profiler
{

	struct SavedData
	{
		float dataMs[10];
	};


	PL::Profiler mainProfiler;

	void startFrame();

	void endFrame();

	void startSubProfile(char *c);
	void endSubProfile(char *c);

	void setSubProfileManually(char *c, PL::ProfileRezults rezults);

	std::unordered_map<char *, PL::Profiler> subProfiles;

	void displayPlot();

	std::deque<SavedData> history;

};