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
	data.dataMs[index++] = mainProfiler.rezult.timeSeconds / 1000.f;

	float accumulatedData = 0;

	for (auto &i : subProfiles)
	{
		float accumulated = 0;
		permaAssert(index != (sizeof(data.dataMs) / sizeof(data.dataMs[0])) );
		data.dataMs[index++] = i.second.end().timeSeconds / 1000.f + accumulated;
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

float plotGetter(const void *data, int index)
{
	return ((float*)data)[index];
}

void Profiler::displayPlot()
{

	void *datas[10] = {};
	static_assert( (sizeof(datas)/sizeof(datas[0])) == (sizeof(SavedData::dataMs) / sizeof(SavedData::dataMs[0])) );

	datas[0] = 
	for (int i = 1; i < subProfiles.size() + i; i++)
	{
		
	}
	

	float data1[10] = {0.4,0.5,0.6,0.7,0.6,0.5,0.4,0.5,0.6,0.7};
	float data2[10] = {0.3,0.3,0.2,0.2,0.3,0.3,0.2,0.2,0.3,0.3};

	//void *datas[2] = {data1, data2};

	const char *names[2] = {"table1", "table2"};
	const ImColor colors[2] = {{1.0f, 1.0f, 0.5f, 1.0f}, {0.7f, 1.0f, 1.0f, 1.0f}};
	ImGui::PlotMultiHistograms("nice plot",  // label
		2,            // num_hists,
		names,        // names,
		colors,       // colors,
		plotGetter, // getter
		datas,		// datas,
		10,           // values_count,
		FLT_MAX,      // scale_min,
		FLT_MAX,      // scale_max,
		ImVec2(256.0, 17.0f)  // graph_size
	);



}
