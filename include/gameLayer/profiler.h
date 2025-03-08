#pragma once
#include <profilerLib.h>
#include <unordered_map>
#include <deque>
#include <string>
#include <vector>
#include <cstring>


struct GPUProfiler
{
	std::vector<unsigned int> queryObjects;
	std::vector<std::string> queryNames;
	std::vector<bool> queryResults;
	std::vector<float> queryTimersMs;
	int currentQuery = 0;

	void init(int maxQueries = 16);

	void startFrame();

	void cleanup();

	void start(std::string name);

	void end();

	void getResults();
};


struct CharEquals
{
	bool operator()(const char * &__x, const char *&__y) const
	{
		return strcmp(__x, __y) == 0;
	}
};

struct Profiler
{

	constexpr static int GPU_PROFILE_FRAMES = 5;
	int currentGpuProfilerIndex = 0;

	GPUProfiler gpuProfiler[5] = {};

	bool gpuProfilingEnabeled = 0;
	void initGPUProfiler();

	bool pause = 0;

	struct SavedData
	{
		float dataMs[16]; //todo magic number here
		float dataMsReal[16];
	};


	PL::Profiler mainProfiler;

	void startFrame();

	void endFrame();

	void startSubProfile(const char *c);
	void endSubProfile(const char *c);

	void setSubProfileManually(char *c, PL::ProfileRezults rezults);

	std::unordered_map<std::string, PL::Profiler> subProfiles;

	void displayPlot(const char *mainPlotName, float scale = 32);

	std::deque<SavedData> history;

};
