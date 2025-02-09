#pragma once

//note this should be included last
#define NOMINMAX
#include "config.h"
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <cstring>

inline size_t constexpr KB(size_t x) { return x * 1024ull; }
inline size_t constexpr MB(size_t x) { return KB(x) * 1024ull; }
inline size_t constexpr GB(size_t x) { return MB(x) * 1024ull; }
inline size_t constexpr TB(size_t x) { return GB(x) * 1024ull; }

inline float constexpr BYTES_TO_KB(size_t x) { return x / 1024.f; }
inline float constexpr BYTES_TO_MB(size_t x) { return BYTES_TO_KB(x) / 1024.f; }
inline float constexpr BYTES_TO_GB(size_t x) { return BYTES_TO_MB(x) / 1024.f; }


#define REMOVE_IMGUI 0



#ifdef _WIN32
	
void assertFuncProduction(
	const char *expression,
	const char *file_name,
	unsigned const line_number,
	const char *comment = "---");
	
void assertFuncInternal(
	const char *expression,
	const char *file_name,
	unsigned const line_number,
	const char *comment = "---");
	
	#if INTERNAL_BUILD == 1
	
		#define permaAssert(expression) (void)(											\
					(!!(expression)) ||												\
					(assertFuncInternal(#expression, __FILE__, (unsigned)(__LINE__)), 0)	\
				)
		
		#define permaAssertComment(expression, comment) (void)(								\
					(!!(expression)) ||														\
					(assertFuncInternal(#expression, __FILE__, (unsigned)(__LINE__), comment), 1)\
				)
		
	#else
	
		#define permaAssert(expression) (void)(											\
					(!!(expression)) ||												\
					(assertFuncProduction(#expression, __FILE__, (unsigned)(__LINE__)), 0)	\
				)
		
		#define permaAssertComment(expression, comment) (void)(								\
					(!!(expression)) ||														\
					(assertFuncProduction(#expression, __FILE__, (unsigned)(__LINE__), comment), 1)	\

				)
		
	#endif
	
	

#else //linux or others
	
void assertFuncProduction(
	const char *expression,
	const char *file_name,
	unsigned const line_number,
	const char *comment = "---");

void assertFuncInternal(
	const char *expression,
	const char *file_name,
	unsigned const line_number,
	const char *comment = "---");

	#if INTERNAL_BUILD == 1
	
		#define permaAssert(expression) (void)(											\
					(!!(expression)) ||												\
					(assertFuncInternal(#expression, __FILE__, (unsigned)(__LINE__)), 0)	\
				)
		
		#define permaAssertComment(expression, comment) (void)(								\
					(!!(expression)) ||														\
					(assertFuncInternal(#expression, __FILE__, (unsigned)(__LINE__), comment), 1)\
				)
		
	#else
	
		#define permaAssert(expression) (void)(											\
					(!!(expression)) ||												\
					(assertFuncProduction(#expression, __FILE__, (unsigned)(__LINE__)), 0)	\
				)
		
		#define permaAssertComment(expression, comment) (void)(								\
					(!!(expression)) ||														\
					(assertFuncProduction(#expression, __FILE__, (unsigned)(__LINE__), comment), 1)	\

				)
		
	#endif


#endif



#include <functional>

struct Defer
{
public:
	explicit Defer(std::function<void()> func): func_(std::move(func)) {}
	~Defer() { func_(); }

	std::function<void()> func_;
};

#define CONCATENATE_DEFFER(x, y) x##y
#define MAKE_UNIQUE_VAR_DEFFER(x, y) CONCATENATE_DEFFER(x, y)
#define defer(func) Defer MAKE_UNIQUE_VAR_DEFFER(_defer_, __COUNTER__)(func)