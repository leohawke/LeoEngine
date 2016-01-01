#ifndef IndePlatform_DebugOutput_hpp
#define IndePlatform_DebugOutput_hpp

#include "leoint.hpp"
namespace leo {
	enum class stdio_type { istream, ostream, estream };

	enum class record_level : uint8
	{
		Emergent = 0x00,
		Alert = 0x20,
		Critical = 0x40,
		Err = 0x60,
		Warning = 0x80,
		Notice = 0xA0,
		Informative = 0xC0,
		Debug = 0xE0
	};

	void RedirectStdIO(stdio_type, char* target);

#if defined(DEBUG)
	namespace details{
	struct  debug_helper
	{
		enum class output_type { console, std, visualstudio } which = output_type::visualstudio;
		
		void operator()(const wchar_t* format, ...);

		void operator()(const char* format, ...);

		void operator()(record_level level, const char* format, ...);

		static debug_helper global_debug;
	};
	}


#if defined(DEBUG_TO_CONSOLE)
	leo::details::debug_helper::global_debug.which = leo::details::debug_helper::output_type::console;
#elif defined(DEBUG_TO_STD)
	leo::details::debug_helper::global_debug.which = leo::details::debug_helper::output_type::std;
#endif
#else
	namespace details {
		//todo : impl this
		struct  record_helper
		{

			void operator()(const wchar_t* format, ...);

			void operator()(const char* format, ...);

			void operator()(record_level level, const char* format, ...);

			static record_helper global_debug;
		};
	}
#endif
}

#ifndef DebugPrintf
#if defined(DEBUG)
#define DebugPrintf leo::details::debug_helper::global_debug
#else
#define DebugPrintf(...)
#endif
#endif


#ifndef RecordPrintf
#if defined(DEBUG)
#define RecordPrintf DebugPrintf
#else
#define RecordPrintf(...) {__VA_ARGS__;}
#endif
#endif

#ifndef LTraceDe
#if defined(DEBUG)
#define LTraceDe(level,...) DebugPrintf(__VA_ARGS__)
#else
#define LTraceDe(level,...) level
#endif
#endif

#endif
