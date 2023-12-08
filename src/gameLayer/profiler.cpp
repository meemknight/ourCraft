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
	int index = 0;
	data.dataMs[index++] = mainProfiler.rezult.timeSeconds * 1000.f;

	float accumulatedData = 0;

	for (auto &i : subProfiles)
	{
		float accumulated = 0;
		permaAssert(index != (sizeof(data.dataMs) / sizeof(data.dataMs[0])) );
		data.dataMs[index++] = i.second.end().timeSeconds * 1000.f + accumulated;
		accumulated = data.dataMs[index++];
	}

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

void Profiler::startSubProfile(char *c)
{

	subProfiles[c].start();

}

void Profiler::endSubProfile(char *c)
{

	auto it = subProfiles.find(c);

	if (it == subProfiles.end())
	{
		permaAssertComment(it != subProfiles.end(), "wrong profile name");
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

	//float *tableData = ((float**)data)[tableIndex];
	//return tableData[index];
}

void Profiler::displayPlot()
{


	//for (int i = 1; i < subProfiles.size() + i; i++)
	//{
	//	
	//}
	

	//float data1[10] = {0.4,0.5,0.6,0.7,0.6,0.5,0.4,0.5,0.6,0.7};
	//float data2[10] = {0.3,0.3,0.2,0.2,0.3,0.3,0.2,0.2,0.3,0.3};

	//void *datas[2] = {data1, data2};

	const char *names[4] = {"Main Table", "table2","table3","table4"}; //todo

	const ImColor colors[4] = {{1.0f, 1.0f, 0.5f, 1.0f}, 
		{0.7f, 1.0f, 1.0f, 1.0f},
		{0.0f, 1.0f, 0.0f, 1.0f},
		{0.2f, 0.1f, 1.0f, 1.0f},
	}; //todo


	ImGui::PlotMultiHistograms("nice plot",  // label
		subProfiles.size() + 1,	// num_hists,
		names,					// names,
		colors,       // colors,
		plotGetter, // getter
		&history,		// datas,
		history.size(),           // values_count,
		0.f,      // scale_min,
		32.f,      // scale_max,
		ImVec2(256.0, 17.0f)  // graph_size
	);



}
