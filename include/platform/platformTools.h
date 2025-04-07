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

		
		#define devOnlyAssert(expression) (void)(											\
					(!!(expression)) ||												\
					(assertFuncInternal(#expression, __FILE__, (unsigned)(__LINE__)), 0)	\
				)

		#define devOnlyAssertComment(expression, comment) (void)(								\
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

		#define devOnlyAssert(expression) 

		#define devOnlyAssertComment(expression, comment) 
		
	#endif
	
	



#include <functional>

//raii stuff, it will basically call the function that you pass to it be called at scope end, usage: defer(func());
struct DeferImpl
{
public:
	explicit DeferImpl(std::function<void()> func): func_(std::move(func)) {}
	~DeferImpl() { func_(); }

	std::function<void()> func_;
};

#define CONCATENATE_DEFFER(x, y) x##y
#define MAKE_UNIQUE_VAR_DEFFER(x, y) CONCATENATE_DEFFER(x, y)
#define defer(func) DeferImpl MAKE_UNIQUE_VAR_DEFFER(_defer_, __COUNTER__)(func)


#if INTERNAL_BUILD == 1
	#define permaAssertDevelopement permaAssert
	#define permaAssertCommentDevelopement permaAssertComment

#else
	#define permaAssertDevelopement
	#define permaAssertCommentDevelopement
#endif