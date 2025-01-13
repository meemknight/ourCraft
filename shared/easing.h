#pragma once
#include <cmath>
#include <utility>


float lerp(float a, float b, float r);

float linearRemap(float val, float fromMin, float fromMax, float toMin, float toMax);


struct EasingEngine
{


	float timer = 0;
	float deltaTime = 0;
	float runDeltaTime = 0;
	float result_ = 0;
	bool hasRestarted = 0;

	EasingEngine &update(float deltaTime)
	{
		this->deltaTime += deltaTime;
		this->runDeltaTime = this->deltaTime;
		this->result_ = 0;
		hasRestarted = 0;
		return *this;
	}

	EasingEngine &constant(float length, float val = 0)
	{
		if (runDeltaTime == 0) { return *this; }

		if (runDeltaTime > length)
		{
			runDeltaTime -= length;
			result_ += val;
		}
		else
		{
			//float ratio = runDeltaTime / length;
			result_ += val;
			runDeltaTime = 0;
		}

		return *this;
	}

	EasingEngine &goTowards(float length, float val = 0)
	{
		if (runDeltaTime == 0) { return *this; }

		if (runDeltaTime > length)
		{
			runDeltaTime -= length;
			result_ = val;
		}
		else
		{
			float ratio = runDeltaTime / length;
			result_ = lerp(result_, val, ratio);
			runDeltaTime = 0;
		}

		return *this;
	}

	EasingEngine &linear(float length, float start = 0, float end = 1)
	{
		if (runDeltaTime == 0) { return *this; }

		if (runDeltaTime > length)
		{
			runDeltaTime -= length;
			result_ += end;
		}
		else
		{
			float ratio = runDeltaTime / length;
			result_ += linearRemap(ratio, 0, 1, start, end);
			runDeltaTime = 0;
		}
		
		return *this;
	}

	EasingEngine &sin(float length, float speed = 1, float start = 0, float end = 1)
	{
		if (runDeltaTime == 0) { return *this; }

		if (runDeltaTime > length)
		{
			runDeltaTime -= length;
			result_ += linearRemap((std::sinf(length * speed) + 1.f)/2.f, 0, 1, start, end);
		}
		else
		{
			result_ += linearRemap((std::sinf(runDeltaTime * speed) + 1.f)/2.f, 0, 1, start, end);
			runDeltaTime = 0;
		}

		return *this;
	}

	EasingEngine &clamp(float min = 0, float max = 1)
	{
		result_ = std::min(std::max(result_, min), max);
		return *this;
	}


	//goes up and down using sin
	EasingEngine &burst(float length, float start = 0, float end = 1, float power = 1)
	{
		if (runDeltaTime == 0) { return *this; }

		if (runDeltaTime > length)
		{
			runDeltaTime -= length;
			result_ += start;
		}
		else
		{
			float ratio = runDeltaTime / length;
			result_ += linearRemap(std::powf(std::sinf(ratio*3.1415926), power)
				, 0, 1, start, end);
			runDeltaTime = 0;
		}

		return *this;
	}

	float result()
	{
		if (runDeltaTime != 0) { deltaTime = 0; hasRestarted = true; } //reset animation
		return result_;
	}


};
