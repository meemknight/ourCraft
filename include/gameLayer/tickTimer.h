#pragma once


constexpr static int targetTicksPerSeccond = 20;


inline int fromSeccondsToTick(float secconds) { return secconds * targetTicksPerSeccond; }
inline float fromTicksToSecconds(int ticks) { return (float)ticks / targetTicksPerSeccond; }
