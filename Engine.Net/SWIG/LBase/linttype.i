%include "inttypes.i"

%{
#include "..\..\LBase\linttype.hpp"
using namespace leo::inttype;
%}

typedef uint8_t uint8 ;
typedef int8_t  int8;
typedef uint16_t uint16;
typedef  int16_t int16; 
typedef  uint32_t uint32;
typedef  int32_t int32; 
typedef uint64_t uint64;
typedef  int64_t int64;

namespace leo{
inline namespace inttype {
		using uint8 = std::uint8_t;
		using int8 = std::int8_t;
		using uint16 = std::uint16_t;
		using int16 = std::int16_t;
		using uint32 = std::uint32_t;
		using int32 = std::int32_t;
		using uint64 = std::uint64_t;
		using int64 = std::int64_t;
	}
}

