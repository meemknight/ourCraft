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

	#include <Windows.h>
	
	inline void assertFuncProduction(
		const char *expression,
		const char *file_name,
		unsigned const line_number,
		const char *comment = "---")
	{
	
		char c[1024] = {};
	
		sprintf(c,
			"Assertion failed\n\n"
			"File:\n"
			"%s\n\n"
			"Line:\n"
			"%u\n\n"
			"Expresion:\n"
			"%s\n\n"
			"Comment:\n"
			"%s"
			"\n\nPlease report this error to the developer.",
			file_name,
			line_number,
			expression,
			comment
		);
	
		int const action = MessageBox(0, c, "Platform Layer", MB_TASKMODAL
			| MB_ICONHAND | MB_OK | MB_SETFOREGROUND);
	
		switch (action)
		{
			case IDOK: // Abort the program:
			{
				raise(SIGABRT);
	
				// We won't usually get here, but it's possible that a user-registered
				// abort handler returns, so exit the program immediately.  Note that
				// even though we are "aborting," we do not call abort() because we do
				// not want to invoke Watson (the user has already had an opportunity
				// to debug the error and chose not to).
				_exit(3);
			}
			default:
			{
				_exit(3);
			}
		}
	
	}
	
	inline void assertFuncInternal(
		const char *expression,
		const char *file_name,
		unsigned const line_number,
		const char *comment = "---")
	{
	
		char c[1024] = {};
	
		sprintf(c,
			"Assertion failed\n\n"
			"File:\n"
			"%s\n\n"
			"Line:\n"
			"%u\n\n"
			"Expresion:\n"
			"%s\n\n"
			"Comment:\n"
			"%s"
			"\n\nPress retry to debug.",
			file_name,
			line_number,
			expression,
			comment
		);
	
		int const action = MessageBox(0, c, "Platform Layer", MB_TASKMODAL
			| MB_ICONHAND | MB_ABORTRETRYIGNORE | MB_SETFOREGROUND);
	
		switch (action)
		{
			case IDABORT: // Abort the program:
			{
				raise(SIGABRT);
	
				// We won't usually get here, but it's possible that a user-registered
				// abort handler returns, so exit the program immediately.  Note that
				// even though we are "aborting," we do not call abort() because we do
				// not want to invoke Watson (the user has already had an opportunity
				// to debug the error and chose not to).
				_exit(3);
			}
			case IDRETRY: // Break into the debugger then return control to caller
			{
				__debugbreak();
				return;
			}
			case IDIGNORE: // Return control to caller
			{
				return;
			}
			default: // This should not happen; treat as fatal error:
			{
				abort();
			}
		}
	
	}
	
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
	
	inline void assertFuncProduction(
		const char *expression,
		const char *file_name,
		unsigned const line_number,
		const char *comment = "---")
	{

		raise(SIGABRT);
	
	}

	inline void assertFuncInternal(
		const char* expression,
		const char* file_name,
		unsigned const line_number,
		const char* comment = "---")
	{

		raise(SIGABRT);

	}


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

