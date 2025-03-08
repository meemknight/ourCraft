#include <glad/glad.h>
#include <profiler.h>
#include <multiPlot.h>
#include <platformTools.h>
#include <iostream>


#if REMOVE_IMGUI == 0
#include <imgui.h>
#endif

void Profiler::initGPUProfiler()
{
	if (REMOVE_IMGUI) { return; }

	for (int i = 0; i < GPU_PROFILE_FRAMES; i++)
	{
		gpuProfiler[i].init(16);
	}

	gpuProfilingEnabeled = 1;
}

void Profiler::startFrame()
{
	if (REMOVE_IMGUI) { return; }

	if (gpuProfilingEnabeled)
	{
		currentGpuProfilerIndex++;
		if (currentGpuProfilerIndex >= GPU_PROFILE_FRAMES)
		{
			currentGpuProfilerIndex = 0;
		}

		gpuProfiler[currentGpuProfilerIndex].startFrame();
	}
	else
	{
		mainProfiler.start();
	}
}

//todo
void Profiler::endFrame()
{
	if (REMOVE_IMGUI) { return; }

	SavedData data = {};

	if (gpuProfilingEnabeled)
	{

		int readProfiler = currentGpuProfilerIndex+1;
		if (readProfiler >= GPU_PROFILE_FRAMES) { readProfiler = 0; }

		gpuProfiler[readProfiler].getResults();

		subProfiles.clear();

		for (int i = 0; i < gpuProfiler[readProfiler].currentQuery; i++)
		{
			PL::Profiler pl;
			pl.rezult.timeSeconds = gpuProfiler[readProfiler].queryTimersMs[i] / 1000.f;
			subProfiles.insert({gpuProfiler[readProfiler].queryNames[i], pl});


			if (!gpuProfiler[readProfiler].queryResults[i])
			{
				//std::cout << "no...\n";
			}

		}
		
		float accumulatedData = 0;

		int index = 1;
		float accumulated = 0;

		bool pauseNextFrame = 0;

		for (auto &i : subProfiles)
		{
			//permaAssert(index != (sizeof(data.dataMs) / sizeof(data.dataMs[0])));

			int position = subProfiles.size() - (index++) + 1;

			data.dataMsReal[position] = i.second.end().timeSeconds * 1000.f;
			data.dataMs[position] = data.dataMsReal[position] + accumulated;

			accumulated = data.dataMs[position];

			//if (accumulated > 40.0)
			//{
			//	pauseNextFrame = true;
			//}
		}

		data.dataMsReal[0] = 0;

		for (int i = 0; i < subProfiles.size(); i++)
		{
			data.dataMsReal[0] += data.dataMsReal[i+1];
		}

		data.dataMs[0] = data.dataMsReal[0];


	}
	else
	{

		mainProfiler.end();
		data.dataMs[0] = mainProfiler.rezult.timeSeconds * 1000.f;
		data.dataMsReal[0] = data.dataMs[0];
	
		float accumulatedData = 0;

		int index = 0;
		float accumulated = 0;

		bool pauseNextFrame = 0;

		for (auto &i : subProfiles)
		{
			permaAssert(index != (sizeof(data.dataMs) / sizeof(data.dataMs[0])));

			int position = subProfiles.size() - (index++);

			data.dataMsReal[position] = i.second.end().timeSeconds * 1000.f;
			data.dataMs[position] = data.dataMsReal[position] + accumulated;

			accumulated = data.dataMs[position];

			if (accumulated > 40.0)
			{
				pauseNextFrame = true;
			}
		}
	

		//if (pauseNextFrame) { pause = true; }
	}

	if (!pause)
	{

		constexpr int frames = 80;
		if (gpuProfilingEnabeled)
		{
			if (history.size())
			{
				for (int i = 0; i < GPU_PROFILE_FRAMES - 1; i++)
				{
					history.pop_back();
				}
			}

			if (history.size() < 80 - (GPU_PROFILE_FRAMES-1))
			{
				history.push_back(data);
			}
			else
			{
				history.pop_front();
				history.push_back(data);
			}

			SavedData dataNotAvailable = {};
			if (history.size())
			{
				for (int i = 0; i < GPU_PROFILE_FRAMES - 1; i++)
				{
					history.push_back(dataNotAvailable);
				}
			}
		}
		else
		{
			if (history.size() < 80)
			{
				history.push_back(data);
			}
			else
			{
				history.pop_front();
				history.push_back(data);
			}
		}

	
	}

}

//
void Profiler::startSubProfile(const char *c)
{
	if (REMOVE_IMGUI) { return; }

	if (gpuProfilingEnabeled)
	{
		gpuProfiler[currentGpuProfilerIndex].start(c);
		glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, c);
	}
	else
	{
		subProfiles[c].start();
	}
}

void Profiler::endSubProfile(const char *c)
{
	if (REMOVE_IMGUI) { return; }

	if (gpuProfilingEnabeled)
	{
		permaAssertComment(c == gpuProfiler[currentGpuProfilerIndex].queryNames[gpuProfiler[currentGpuProfilerIndex].currentQuery], 
			"Inconsistent start end sub profiler for GPU profiling");
		gpuProfiler[currentGpuProfilerIndex].end();

		glPopDebugGroup();
	}
	else
	{
		auto it = subProfiles.find(c);

		if (it == subProfiles.end())
		{
			//permaAssertComment(it != subProfiles.end(), "wrong profile name");
		}
		else
		{
			it->second.end();
		}

	}

	
}

void Profiler::setSubProfileManually(char *c, PL::ProfileRezults rezults)
{
	if (REMOVE_IMGUI) { return; }

	assert(!gpuProfiler && "This function aint compatible with the gpu profiler!");

	subProfiles[c].end();
	subProfiles[c].rezult = rezults;
}

float plotGetter(const void *data, int index, int tableIndex)
{
	std::deque<Profiler::SavedData> *history = (std::deque<Profiler::SavedData>*)data;

	return ((*history)[index]).dataMs[tableIndex];
}

float plotGetterReal(const void *data, int index, int tableIndex)
{
	std::deque<Profiler::SavedData> *history = (std::deque<Profiler::SavedData>*)data;

	return ((*history)[index]).dataMsReal[tableIndex];
}

void Profiler::displayPlot(const char *mainPlotName, float scale)
{
	if (history.empty()) { return; }

#if REMOVE_IMGUI == 0

	ImGui::PushID(mainPlotName);

	ImGui::Checkbox("Pause", &pause);
	ImGui::SameLine();

	const char *names[16] = {mainPlotName}; //todo

	int counter = 0;
	for (auto &p : subProfiles)
	{
		names[subProfiles.size() - (counter++)] = p.first.c_str();
	}

	const ImColor colors[16] = {
		{1.0f, 1.0f, 1.0f, 1.0f},
		{1.0f, 1.0f, 0.5f, 1.0f},
		{0.1f, 0.3f, 0.9f, 1.0f},
		{0.0f, 1.0f, 0.0f, 1.0f},
		{0.7f, 0.6f, 1.0f, 1.0f},
		{0.6f, 0.0f, 0.3f, 1.0f},
		{0.9f, 0.1f, 0.0f, 1.0f},
		{0.0f, 0.5f, 0.5f, 1.0f},
		{1.0f, 0.0f, 1.0f, 1.0f},
		{1.0f, 0.0f, 0.0f, 1.0f},
		{0.3f, 1.0f, 0.3f, 1.0f},
		{0.3f, 0.3f, 1.0f, 1.0f},
		{1.0f, 0.5f, 1.0f, 1.0f},
		{1.0f, 0.5f, 0.0f, 1.0f},
		{0.3f, 0.9f, 0.5f, 1.0f},
		{0.5f, 0.5f, 1.0f, 1.0f},
	}; //todo

	ImGui::Text(mainPlotName);


	ImGui::PlotMultiHistograms(mainPlotName,  // label
		subProfiles.size() + 1,	// num_hists,
		names,					// names,
		colors,       // colors,
		plotGetter, // getter
		plotGetterReal, // getter
		&history,		// datas,
		history.size(),           // values_count,
		0.f,      // scale_min,
		scale,      // scale_max,
		ImVec2(512.0, 117.0f)  // graph_size
	);
	

	ImGui::Text("Average:");
	for (int index = 0; index<subProfiles.size() + 1; index++)
	{
		float average = 0;
		for (int i = 0; i < history.size(); i++)
		{
			average += history[i].dataMsReal[index];
		}
		average /= history.size();

		ImGui::ColorButton("X", colors[index], ImGuiColorEditFlags_NoInputs);
		ImGui::SameLine();
		ImGui::Text("%s: %f ms", names[index], average);
	}


	ImGui::PopID();
#endif

}

void GPUProfiler::init(int maxQueries)
{
	queryObjects.resize(maxQueries);
	glGenQueries(maxQueries, queryObjects.data());
	queryNames.resize(maxQueries);
	queryResults.resize(maxQueries, false);
	queryTimersMs.resize(maxQueries, 0);
}

void GPUProfiler::startFrame()
{
	for (auto &c : queryNames) { c = {}; }
	for (int i = 0; i < queryResults.size(); i++) { queryResults[i] = 0; }
	//for (auto &c : queryResults) { c = {}; }
	for (auto &c: queryTimersMs) { c = {}; }
	currentQuery = 0;
}

void GPUProfiler::cleanup()
{
	glDeleteQueries(queryObjects.size(), queryObjects.data());
}

void GPUProfiler::start(std::string name)
{
	if (currentQuery >= queryObjects.size())
	{
		permaAssertComment(0, "Exceeded maximum number of queries!");
		return;
	}
	queryNames[currentQuery] = std::move(name);
	glBeginQuery(GL_TIME_ELAPSED, queryObjects[currentQuery]);
}

void GPUProfiler::end()
{
	if (currentQuery < queryObjects.size())
	{
		glEndQuery(GL_TIME_ELAPSED);
		currentQuery++;
	}
}

inline void GPUProfiler::getResults()
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
