#pragma once

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"

namespace ImGui
{

    ImU32 InvertColorU32(ImU32 in);

    void PlotMultiEx(
        ImGuiPlotType plot_type,
        const char *label,
        int num_datas,
        const char **names,
        const ImColor *colors,
        float(*getter)(const void *data, int idx, int tableIndex),
        float(*getterToolTip)(const void *data, int idx, int tableIndex),
        const void *datas,
        int values_count,
        float scale_min,
        float scale_max,
        ImVec2 graph_size);

    void PlotMultiLines(
        const char *label,
        int num_datas,
        const char **names,
        const ImColor *colors,
        float(*getter)(const void *data, int idx, int tableIndex),
        float(*getterToolTip)(const void *data, int idx, int tableIndex),
        const void *datas,
        int values_count,
        float scale_min,
        float scale_max,
        ImVec2 graph_size);

    void PlotMultiHistograms(
        const char *label,
        int num_hists,
        const char **names,
        const ImColor *colors,
        float(*getter)(const void *data, int idx, int tableIndex),
        float(*getterToolTip)(const void *data, int idx, int tableIndex),
        const void *datas,
        int values_count,
        float scale_min,
        float scale_max,
        ImVec2 graph_size);

} // namespace ImGui
