%include "LBase\linttype.i"

%{
#include "..\..\Engine\Render\IFormat.hpp"
%}

#define EC_R 0UL
#define EC_G 1UL
#define EC_B 2UL
#define EC_A 3UL
#define EC_D 4UL
#define EC_S 5UL
#define EC_BC  6UL
#define EC_E  7UL
#define EC_ETC 8UL

#define ECT_UNorm 0UL
#define	ECT_SNorm 1UL
#define	ECT_UInt 2UL
#define	ECT_SInt 3UL
#define	ECT_Float 4UL
#define	ECT_UNorm_SRGB 5UL
#define	ECT_Typeless 6UL
#define	ECT_SharedExp 7UL

#define MakeElementFormat4( ch0,  ch1,  ch2,  ch3, ch0_size,  ch1_size,  ch2_size,  ch3_size, ch0_type,  ch1_type,  ch2_type,  ch3_type) \
	(ch0 << 0) | (ch1 << 4) | (ch2 << 8) | (ch3 << 12) \
				| (ch0_size << 16) | (ch1_size << 22) | (ch2_size << 28) | (ch3_size << 34) \
				| (ch0_type << 40) | (ch1_type << 44) | (ch2_type << 48) | (ch3_type << 52) \

#define MakeElementFormat3( ch0,  ch1,  ch2,ch0_size,  ch1_size,  ch2_size,ch0_type,  ch1_type,  ch2_type) MakeElementFormat4(ch0, ch1, ch2, 0UL, ch0_size, ch1_size, ch2_size, 0UL, ch0_type, ch1_type, ch2_type, 0UL)
#define MakeElementFormat2(ch0,  ch1, ch0_size,  ch1_size,ch0_type,  ch1_type) MakeElementFormat3(ch0, ch1, 0UL, ch0_size, ch1_size, 0UL, ch0_type, ch1_type, 0UL)
#define MakeElementFormat1(ch0, ch0_size,ch0_type) MakeElementFormat2(ch0, 0UL, ch0_size, 0UL, ch0_type, 0UL)


%typemap(csbase) EFormat "ulong"
enum class EFormat : uint64 {
// Unknown element format.
EF_Unknown = 0,

// 8-bit element format, all bits alpha.
EF_A8 = MakeElementFormat1(EC_A, 8UL, ECT_UNorm),

// 16-bit element format, 5 bits for red, 6 bits for green and 5 bits for blue.
EF_R5G6B5 = MakeElementFormat3(EC_R, EC_G, EC_B, 5, 6, 5, ECT_UNorm, ECT_UNorm, ECT_UNorm),
// 16-bit element format, 1 bits for alpha, 5 bits for red, green and blue.
EF_A1RGB5 = MakeElementFormat4(EC_A, EC_R, EC_G, EC_B, 1, 5, 5, 5, ECT_UNorm, ECT_UNorm, ECT_UNorm, ECT_UNorm),
// 16-bit element format, 4 bits for alpha, red, green and blue.
EF_ARGB4 = MakeElementFormat4(EC_A, EC_R, EC_G, EC_B, 4, 4, 4, 4, ECT_UNorm, ECT_UNorm, ECT_UNorm, ECT_UNorm),

// 8-bit element format, 8 bits for red.
EF_R8 = MakeElementFormat1(EC_R, 8UL, ECT_UNorm),
// 8-bit element format, 8 bits for signed red.
EF_SIGNED_R8 = MakeElementFormat1(EC_R, 8UL, ECT_SNorm),
// 16-bit element format, 8 bits for red, green.
EF_GR8 = MakeElementFormat2(EC_G, EC_R, 8UL, 8UL, ECT_UNorm, ECT_UNorm),
// 16-bit element format, 8 bits for signed red, green.
EF_SIGNED_GR8 = MakeElementFormat2(EC_G, EC_R, 8UL, 8UL, ECT_SNorm, ECT_SNorm),
// 24-bit element format, 8 bits for red, green and blue.
EF_BGR8 = MakeElementFormat3(EC_B, EC_G, EC_R, 8UL, 8UL, 8UL, ECT_UNorm, ECT_UNorm, ECT_UNorm),
// 24-bit element format, 8 bits for signed red, green and blue.
EF_SIGNED_BGR8 = MakeElementFormat3(EC_B, EC_G, EC_R, 8UL, 8UL, 8UL, ECT_SNorm, ECT_SNorm, ECT_SNorm),
// 32-bit element format, 8 bits for alpha, red, green and blue.
EF_ARGB8 = MakeElementFormat4(EC_A, EC_R, EC_G, EC_B, 8UL, 8UL, 8UL, 8UL, ECT_UNorm, ECT_UNorm, ECT_UNorm, ECT_UNorm),
// 32-bit element format, 8 bits for alpha, red, green and blue.
EF_ABGR8 = MakeElementFormat4(EC_A, EC_B, EC_G, EC_R, 8UL, 8UL, 8UL, 8UL, ECT_UNorm, ECT_UNorm, ECT_UNorm, ECT_UNorm),
// 32-bit element format, 8 bits for signed alpha, red, green and blue.
EF_SIGNED_ABGR8 = MakeElementFormat4(EC_A, EC_B, EC_G, EC_R, 8UL, 8UL, 8UL, 8UL, ECT_SNorm, ECT_SNorm, ECT_SNorm, ECT_SNorm),
// 32-bit element format, 2 bits for alpha, 10 bits for red, green and blue.
EF_A2BGR10 = MakeElementFormat4(EC_A, EC_B, EC_G, EC_R, 2, 10UL, 10UL, 10UL, ECT_UNorm, ECT_UNorm, ECT_UNorm, ECT_UNorm),
// 32-bit element format, 2 bits for alpha, 10 bits for signed red, green and blue.
EF_SIGNED_A2BGR10 = MakeElementFormat4(EC_A, EC_B, EC_G, EC_R, 2, 10UL, 10UL, 10UL, ECT_SNorm, ECT_SNorm, ECT_SNorm, ECT_SNorm),

// 32-bit element format, 8 bits for alpha, red, green and blue.
EF_R8UI = MakeElementFormat1(EC_R, 8UL, ECT_UInt),
// 32-bit element format, 8 bits for alpha, red, green and blue.
EF_R8I = MakeElementFormat1(EC_R, 8UL, ECT_SInt),
// 16-bit element format, 8 bits for red, green.
EF_GR8UI = MakeElementFormat2(EC_G, EC_R, 8UL, 8UL, ECT_UInt, ECT_UInt),
// 16-bit element format, 8 bits for red, green.
EF_GR8I = MakeElementFormat2(EC_G, EC_R, 8UL, 8UL, ECT_SInt, ECT_SInt),
// 24-bit element format, 8 bits for red, green and blue.
EF_BGR8UI = MakeElementFormat3(EC_B, EC_G, EC_R, 8UL, 8UL, 8UL, ECT_UInt, ECT_UInt, ECT_UInt),
// 24-bit element format, 8 bits for red, green and blue.
EF_BGR8I = MakeElementFormat3(EC_B, EC_G, EC_R, 8UL, 8UL, 8UL, ECT_SInt, ECT_SInt, ECT_SInt),
// 32-bit element format, 8 bits for alpha, red, green and blue.
EF_ABGR8UI = MakeElementFormat4(EC_A, EC_B, EC_G, EC_R, 8UL, 8UL, 8UL, 8UL, ECT_UInt, ECT_UInt, ECT_UInt, ECT_UInt),
// 32-bit element format, 8 bits for signed alpha, red, green and blue.
EF_ABGR8I = MakeElementFormat4(EC_A, EC_B, EC_G, EC_R, 8UL, 8UL, 8UL, 8UL, ECT_SInt, ECT_SInt, ECT_SInt, ECT_SInt),
// 32-bit element format, 2 bits for alpha, 10 bits for red, green and blue.
EF_A2BGR10UI = MakeElementFormat4(EC_A, EC_B, EC_G, EC_R, 2, 10UL, 10UL, 10UL, ECT_UInt, ECT_UInt, ECT_UInt, ECT_UInt),
// 32-bit element format, 2 bits for alpha, 10 bits for red, green and blue.
EF_A2BGR10I = MakeElementFormat4(EC_A, EC_B, EC_G, EC_R, 2, 10UL, 10UL, 10UL, ECT_SInt, ECT_SInt, ECT_SInt, ECT_SInt),

// 16-bit element format, 16 bits for red.
EF_R16 = MakeElementFormat1(EC_R, 16, ECT_UNorm),
// 16-bit element format, 16 bits for signed red.
EF_SIGNED_R16 = MakeElementFormat1(EC_R, 16, ECT_SNorm),
// 32-bit element format, 16 bits for red and green.
EF_GR16 = MakeElementFormat2(EC_G, EC_R, 16, 16, ECT_UNorm, ECT_UNorm),
// 32-bit element format, 16 bits for signed red and green.
EF_SIGNED_GR16 = MakeElementFormat2(EC_G, EC_R, 16, 16, ECT_SNorm, ECT_SNorm),
// 48-bit element format, 16 bits for alpha, blue, green and red.
EF_BGR16 = MakeElementFormat3(EC_B, EC_G, EC_R, 16, 16, 16, ECT_UNorm, ECT_UNorm, ECT_UNorm),
// 48-bit element format, 16 bits for signed alpha, blue, green and red.
EF_SIGNED_BGR16 = MakeElementFormat3(EC_B, EC_G, EC_R, 16, 16, 16, ECT_SNorm, ECT_SNorm, ECT_SNorm),
// 64-bit element format, 16 bits for alpha, blue, green and red.
EF_ABGR16 = MakeElementFormat4(EC_A, EC_B, EC_G, EC_R, 16, 16, 16, 16, ECT_UNorm, ECT_UNorm, ECT_UNorm, ECT_UNorm),
// 64-bit element format, 16 bits for signed alpha, blue, green and red.
EF_SIGNED_ABGR16 = MakeElementFormat4(EC_A, EC_B, EC_G, EC_R, 16, 16, 16, 16, ECT_SNorm, ECT_SNorm, ECT_SNorm, ECT_SNorm),
// 32-bit element format, 32 bits for red.
EF_R32 = MakeElementFormat1(EC_R, 32, ECT_UNorm),
// 32-bit element format, 32 bits for signed red.
EF_SIGNED_R32 = MakeElementFormat1(EC_R, 32, ECT_SNorm),
// 64-bit element format, 16 bits for red and green.
EF_GR32 = MakeElementFormat2(EC_G, EC_R, 32, 32, ECT_UNorm, ECT_UNorm),
// 64-bit element format, 16 bits for signed red and green.
EF_SIGNED_GR32 = MakeElementFormat2(EC_G, EC_R, 32, 32, ECT_SNorm, ECT_SNorm),
// 96-bit element format, 16 bits for alpha, blue, green and red.
EF_BGR32 = MakeElementFormat3(EC_B, EC_G, EC_R, 32, 32, 32, ECT_UNorm, ECT_UNorm, ECT_UNorm),
// 96-bit element format, 16 bits for signed_alpha, blue, green and red.
EF_SIGNED_BGR32 = MakeElementFormat3(EC_B, EC_G, EC_R, 32, 32, 32, ECT_SNorm, ECT_SNorm, ECT_SNorm),
// 128-bit element format, 16 bits for alpha, blue, green and red.
EF_ABGR32 = MakeElementFormat4(EC_A, EC_B, EC_G, EC_R, 32, 32, 32, 32, ECT_UNorm, ECT_UNorm, ECT_UNorm, ECT_UNorm),
// 128-bit element format, 16 bits for signed alpha, blue, green and red.
EF_SIGNED_ABGR32 = MakeElementFormat4(EC_A, EC_B, EC_G, EC_R, 32, 32, 32, 32, ECT_SNorm, ECT_SNorm, ECT_SNorm, ECT_SNorm),

// 16-bit element format, 16 bits for red.
EF_R16UI = MakeElementFormat1(EC_R, 16, ECT_UInt),
// 16-bit element format, 16 bits for signed red.
EF_R16I = MakeElementFormat1(EC_R, 16, ECT_SInt),
// 32-bit element format, 16 bits for red and green.
EF_GR16UI = MakeElementFormat2(EC_G, EC_R, 16, 16, ECT_UInt, ECT_UInt),
// 32-bit element format, 16 bits for signed red and green.
EF_GR16I = MakeElementFormat2(EC_G, EC_R, 16, 16, ECT_SInt, ECT_SInt),
// 48-bit element format, 16 bits for alpha, blue, green and red.
EF_BGR16UI = MakeElementFormat3(EC_B, EC_G, EC_R, 16, 16, 16, ECT_UInt, ECT_UInt, ECT_UInt),
// 48-bit element format, 16 bits for signed alpha, blue, green and red.
EF_BGR16I = MakeElementFormat3(EC_B, EC_G, EC_R, 16, 16, 16, ECT_SInt, ECT_SInt, ECT_SInt),
// 64-bit element format, 16 bits for alpha, blue, green and red.
EF_ABGR16UI = MakeElementFormat4(EC_A, EC_B, EC_G, EC_R, 16, 16, 16, 16, ECT_UInt, ECT_UInt, ECT_UInt, ECT_UInt),
// 64-bit element format, 16 bits for signed alpha, blue, green and red.
EF_ABGR16I = MakeElementFormat4(EC_A, EC_B, EC_G, EC_R, 16, 16, 16, 16, ECT_SInt, ECT_SInt, ECT_SInt, ECT_SInt),
// 32-bit element format, 32 bits for red.
EF_R32UI = MakeElementFormat1(EC_R, 32, ECT_UInt),
// 32-bit element format, 32 bits for signed red.
EF_R32I = MakeElementFormat1(EC_R, 32, ECT_SInt),
// 64-bit element format, 16 bits for red and green.
EF_GR32UI = MakeElementFormat2(EC_G, EC_R, 32, 32, ECT_UInt, ECT_UInt),
// 64-bit element format, 16 bits for signed red and green.
EF_GR32I = MakeElementFormat2(EC_G, EC_R, 32, 32, ECT_SInt, ECT_SInt),
// 96-bit element format, 16 bits for alpha, blue, green and red.
EF_BGR32UI = MakeElementFormat3(EC_B, EC_G, EC_R, 32, 32, 32, ECT_UInt, ECT_UInt, ECT_UInt),
// 96-bit element format, 16 bits for signed_alpha, blue, green and red.
EF_BGR32I = MakeElementFormat3(EC_B, EC_G, EC_R, 32, 32, 32, ECT_SInt, ECT_SInt, ECT_SInt),
// 128-bit element format, 16 bits for alpha, blue, green and red.
EF_ABGR32UI = MakeElementFormat4(EC_A, EC_B, EC_G, EC_R, 32, 32, 32, 32, ECT_UInt, ECT_UInt, ECT_UInt, ECT_UInt),
// 128-bit element format, 16 bits for signed alpha, blue, green and red.
EF_ABGR32I = MakeElementFormat4(EC_A, EC_B, EC_G, EC_R, 32, 32, 32, 32, ECT_SInt, ECT_SInt, ECT_SInt, ECT_SInt),

// 16-bit element format, 16 bits floating-point for red.
EF_R16F = MakeElementFormat1(EC_R, 16, ECT_Float),
// 32-bit element format, 16 bits floating-point for green and red.
EF_GR16F = MakeElementFormat2(EC_G, EC_R, 16, 16, ECT_Float, ECT_Float),
// 32-bit element format, 11 bits floating-point for green and red, 10 bits floating-point for blue.
EF_B10G11R11F = MakeElementFormat3(EC_B, EC_G, EC_R, 10UL, 11UL, 11UL, ECT_Float, ECT_Float, ECT_Float),
// 48-bit element format, 16 bits floating-point for blue, green and red.
EF_BGR16F = MakeElementFormat3(EC_B, EC_G, EC_R, 16, 16, 16, ECT_Float, ECT_Float, ECT_Float),
// 64-bit element format, 16 bits floating-point for alpha, blue, green and red.
EF_ABGR16F = MakeElementFormat4(EC_A, EC_B, EC_G, EC_R, 16, 16, 16, 16, ECT_Float, ECT_Float, ECT_Float, ECT_Float),
// 32-bit element format, 32 bits floating-point for red.
EF_R32F = MakeElementFormat1(EC_R, 32, ECT_Float),
// 64-bit element format, 32 bits floating-point for green and red.
EF_GR32F = MakeElementFormat2(EC_G, EC_R, 32, 32, ECT_Float, ECT_Float),
// 96-bit element format, 32 bits floating-point for blue, green and red.
EF_BGR32F = MakeElementFormat3(EC_B, EC_G, EC_R, 32, 32, 32, ECT_Float, ECT_Float, ECT_Float),
// 128-bit element format, 32 bits floating-point for alpha, blue, green and red.
EF_ABGR32F = MakeElementFormat4(EC_A, EC_B, EC_G, EC_R, 32, 32, 32, 32, ECT_Float, ECT_Float, ECT_Float, ECT_Float),

// BC1 compression element format, DXT1
EF_BC1 = MakeElementFormat1(EC_BC, 1, ECT_UNorm),
// BC1 compression element format, signed DXT1
EF_SIGNED_BC1 = MakeElementFormat1(EC_BC, 1, ECT_SNorm),
// BC2 compression element format, DXT3
EF_BC2 = MakeElementFormat1(EC_BC, 2, ECT_UNorm),
// BC2 compression element format, signed DXT3
EF_SIGNED_BC2 = MakeElementFormat1(EC_BC, 2, ECT_SNorm),
// BC3 compression element format, DXT5
EF_BC3 = MakeElementFormat1(EC_BC, 3, ECT_UNorm),
// BC3 compression element format, signed DXT5
EF_SIGNED_BC3 = MakeElementFormat1(EC_BC, 3, ECT_SNorm),
// BC4 compression element format, 1 channel
EF_BC4 = MakeElementFormat1(EC_BC, 4, ECT_UNorm),
// BC4 compression element format, 1 channel signed
EF_SIGNED_BC4 = MakeElementFormat1(EC_BC, 4, ECT_SNorm),
// BC5 compression element format, 2 channels
EF_BC5 = MakeElementFormat1(EC_BC, 5, ECT_UNorm),
// BC5 compression element format, 2 channels signed
EF_SIGNED_BC5 = MakeElementFormat1(EC_BC, 5, ECT_SNorm),
// BC6 compression element format, 3 channels
EF_BC6 = MakeElementFormat1(EC_BC, 6, ECT_UNorm),
// BC6 compression element format, 3 channels
EF_SIGNED_BC6 = MakeElementFormat1(EC_BC, 6, ECT_SNorm),
// BC7 compression element format, 3 channels
EF_BC7 = MakeElementFormat1(EC_BC, 7, ECT_UNorm),

// ETC1 compression element format
EF_ETC1 = MakeElementFormat1(EC_ETC, 1, ECT_UNorm),
// ETC2 R11 compression element format
EF_ETC2_R11 = MakeElementFormat2(EC_ETC, EC_ETC, 2, 1, ECT_UNorm, ECT_UNorm),
// ETC2 R11 compression element format, signed
EF_SIGNED_ETC2_R11 = MakeElementFormat2(EC_ETC, EC_ETC, 2, 1, ECT_UNorm, ECT_SNorm),
// ETC2 GR11 compression element format
EF_ETC2_GR11 = MakeElementFormat2(EC_ETC, EC_ETC, 2, 2, ECT_UNorm, ECT_UNorm),
// ETC2 GR11 compression element format, signed
EF_SIGNED_ETC2_GR11 = MakeElementFormat2(EC_ETC, EC_ETC, 2, 2, ECT_UNorm, ECT_SNorm),
// ETC2 BGR8 compression element format
EF_ETC2_BGR8 = MakeElementFormat2(EC_ETC, EC_ETC, 2, 3, ECT_UNorm, ECT_UNorm),
// ETC2 BGR8 compression element format. Standard RGB (gamma = 2.2).
EF_ETC2_BGR8_SRGB = MakeElementFormat2(EC_ETC, EC_ETC, 2, 3, ECT_UNorm_SRGB, ECT_UNorm_SRGB),
// ETC2 A1BGR8 compression element format
EF_ETC2_A1BGR8 = MakeElementFormat2(EC_ETC, EC_ETC, 2, 4, ECT_UNorm, ECT_UNorm),
// ETC2 A1BGR8 compression element format. Standard RGB (gamma = 2.2).
EF_ETC2_A1BGR8_SRGB = MakeElementFormat2(EC_ETC, EC_ETC, 2, 4, ECT_UNorm_SRGB, ECT_UNorm_SRGB),
// ETC2 ABGR8 compression element format
EF_ETC2_ABGR8 = MakeElementFormat2(EC_ETC, EC_ETC, 2, 5, ECT_UNorm, ECT_UNorm),
// ETC2 ABGR8 compression element format. Standard RGB (gamma = 2.2).
EF_ETC2_ABGR8_SRGB = MakeElementFormat2(EC_ETC, EC_ETC, 2, 5, ECT_UNorm_SRGB, ECT_UNorm_SRGB),

// 16-bit element format, 16 bits depth
EF_D16 = MakeElementFormat1(EC_D, 16, ECT_UNorm),
// 32-bit element format, 24 bits depth and 8 bits stencil
EF_D24S8 = MakeElementFormat2(EC_D, EC_S, 24, 8UL, ECT_UNorm, ECT_UInt),
// 32-bit element format, 32 bits depth
EF_D32F = MakeElementFormat1(EC_D, 32, ECT_Float),

// 32-bit element format, 8 bits for alpha, red, green and blue. Standard RGB (gamma = 2.2).
EF_ARGB8_SRGB = MakeElementFormat4(EC_A, EC_R, EC_G, EC_B, 8UL, 8UL, 8UL, 8UL, ECT_UNorm_SRGB, ECT_UNorm_SRGB, ECT_UNorm_SRGB, ECT_UNorm_SRGB),
// 32-bit element format, 8 bits for alpha, red, green and blue. Standard RGB (gamma = 2.2).
EF_ABGR8_SRGB = MakeElementFormat4(EC_A, EC_B, EC_G, EC_R, 8UL, 8UL, 8UL, 8UL, ECT_UNorm_SRGB, ECT_UNorm_SRGB, ECT_UNorm_SRGB, ECT_UNorm_SRGB),
// BC1 compression element format. Standard RGB (gamma = 2.2).
EF_BC1_SRGB = MakeElementFormat1(EC_BC, 1, ECT_UNorm_SRGB),
// BC2 compression element format. Standard RGB (gamma = 2.2).
EF_BC2_SRGB = MakeElementFormat1(EC_BC, 2, ECT_UNorm_SRGB),
// BC3 compression element format. Standard RGB (gamma = 2.2).
EF_BC3_SRGB = MakeElementFormat1(EC_BC, 3, ECT_UNorm_SRGB),
// BC4 compression element format. Standard RGB (gamma = 2.2).
EF_BC4_SRGB = MakeElementFormat1(EC_BC, 4, ECT_UNorm_SRGB),
// BC5 compression element format. Standard RGB (gamma = 2.2).
EF_BC5_SRGB = MakeElementFormat1(EC_BC, 5, ECT_UNorm_SRGB),
// BC7 compression element format. Standard RGB (gamma = 2.2).
EF_BC7_SRGB = MakeElementFormat1(EC_BC, 7, ECT_UNorm_SRGB)
};


#undef EC_R
#undef EC_G
#undef EC_B
#undef EC_A
#undef EC_D
#undef EC_S
#undef EC_BC 
#undef EC_E 
#undef EC_ETC

#undef ECT_UNorm
#undef	ECT_SNorm 
#undef	ECT_UInt 
#undef	ECT_SInt
#undef	ECT_Float
#undef	ECT_UNorm_SRGB
#undef	ECT_Typeless
#undef	ECT_SharedExp

%typemap(csbase) EChannel "ulong"
enum EChannel
{
	R = 0UL,
	G = 1UL,
	B = 2UL,
	A = 3UL,
	D = 4UL,
	S = 5UL,
	BC = 6UL,
	E = 7UL,
	ETC = 8UL
};

%typemap(csbase) EChannelType "ulong"
enum EChannelType
{
	UNorm = 0UL,
	SNorm = 1UL,
	UInt = 2UL,
	SInt = 3UL,
	Float = 4UL,
	UNorm_SRGB = 5UL,
	Typeless = 6UL,
	SharedExp = 7UL
};

