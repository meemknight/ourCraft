#include <profiler.h>
#include <multiPlot.h>

void Profiler::startFrame()
{
	mainProfiler.start();
}

void Profiler::endFrame()
{
	mainProfiler.end();

	SavedData data;
	data.dataMs[0] = mainProfiler.rezult.timeSeconds * 1000.f;
	data.dataMsReal[0] = data.dataMs[0];

	float accumulatedData = 0;

	int index = 0;
	float accumulated = 0;
	for (auto &i : subProfiles)
	{
		permaAssert(index != (sizeof(data.dataMs) / sizeof(data.dataMs[0])) );

		int position = subProfiles.size() - (index++);

		data.dataMsReal[position] = i.second.end().timeSeconds * 1000.f;
		data.dataMs[position] = data.dataMsReal[position] + accumulated;
		
		accumulated = data.dataMs[position];
	}

	if (!pause)
	{
		if (history.size() < 30)
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

void Profiler::startSubProfile(char *c)
{

	subProfiles[c].start();

}

void Profiler::endSubProfile(char *c)
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

void Profiler::setSubProfileManually(char *c, PL::ProfileRezults rezults)
{
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

void Profiler::displayPlot(const char *mainPlotName)
{
	if (history.empty()) { return; }

	ImGui::PushID(mainPlotName);

	ImGui::Checkbox("Pause", &pause);
	ImGui::SameLine();

	const char *names[10] = {mainPlotName}; //todo

	int counter = 0;
	for (auto &p : subProfiles)
	{
		names[subProfiles.size() - (counter++)] = p.first.c_str();
	}

	const ImColor colors[10] = {
		{1.0f, 1.0f, 1.0f, 1.0f},
		{1.0f, 1.0f, 0.5f, 1.0f},
		{0.7f, 1.0f, 1.0f, 1.0f},
		{0.0f, 1.0f, 0.0f, 1.0f},
		{0.2f, 0.1f, 1.0f, 1.0f},
		{0.6f, 0.0f, 0.3f, 1.0f},
		{0.9f, 0.1f, 0.0f, 1.0f},
		{0.0f, 1.0f, 1.0f, 1.0f},
		{1.0f, 0.0f, 1.0f, 1.0f},
		{1.0f, 0.0f, 0.0f, 1.0f},
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
		32.f,      // scale_max,
		ImVec2(256.0, 117.0f)  // graph_size
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

		ImGui::ColorButton("X", colors[index]);
		ImGui::SameLine();
		ImGui::Text("%s: %f ms", names[index], average);
	}


	ImGui::PopID();

}
