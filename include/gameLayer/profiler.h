#pragma once
#include <glad/glad.h>
#include <profilerLib.h>
#include <unordered_map>
#include <platformTools.h>
#include <deque>
#include <string>


struct GPUProfiler
{
	std::vector<GLuint> queryObjects;
	std::vector<std::string> queryNames;
	std::vector<bool> queryResults;
	std::vector<float> queryTimersMs;
	int currentQuery = 0;

	void init(int maxQueries = 10);

	void startFrame();

	void cleanup();

	void start(std::string name);

	void end();

	void getResults()
	{
		for (int i = 0; i < currentQuery; ++i)
		{
			if (!queryResults[i])
			{
				GLint available = 0;
				glGetQueryObjectiv(queryObjects[i], GL_QUERY_RESULT_AVAILABLE, &available);
				if (available)
				{
					GLuint64 timeElapsed;
					glGetQueryObjectui64v(queryObjects[i], GL_QUERY_RESULT, &timeElapsed);
					queryTimersMs[i] = timeElapsed / 1.0e6;
					//std::cout << queryNames[i] << ": " << timeElapsed / 1.0e6 << " ms" << std::endl;
					queryResults[i] = true;
				}
				else
				{
					queryTimersMs[i] = false;
				}
			}
		}
	}
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
		float dataMs[10]; //todo magic number here
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