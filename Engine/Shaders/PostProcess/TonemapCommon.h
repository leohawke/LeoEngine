#ifndef TonemapCommon_h
#define TonemapCommon_h 1

#include "Common.h"
#include "ACES.h"
#include "GammaCorrectionCommon.h"

float3 InverseGamma;
float FilmSlope;
float FilmToe;
float FilmShoulder;
float FilmBlackClip;
float FilmWhiteClip;
half3 FilmToneMap( half3 LinearColor ) 
{
	const float3x3 sRGB_2_AP0 = mul( XYZ_2_AP0_MAT, mul( D65_2_D60_CAT, sRGB_2_XYZ_MAT ) );
	const float3x3 sRGB_2_AP1 = mul( XYZ_2_AP1_MAT, mul( D65_2_D60_CAT, sRGB_2_XYZ_MAT ) );

	const float3x3 AP0_2_sRGB = mul( XYZ_2_sRGB_MAT, mul( D60_2_D65_CAT, AP0_2_XYZ_MAT ) );
	const float3x3 AP1_2_sRGB = mul( XYZ_2_sRGB_MAT, mul( D60_2_D65_CAT, AP1_2_XYZ_MAT ) );
	
	const float3x3 AP0_2_AP1 = mul( XYZ_2_AP1_MAT, AP0_2_XYZ_MAT );
	const float3x3 AP1_2_AP0 = mul( XYZ_2_AP0_MAT, AP1_2_XYZ_MAT );
	
	float3 ColorAP1 = LinearColor;
	//float3 ColorAP1 = mul( sRGB_2_AP1, float3(LinearColor) );

	float3 ColorAP0 = mul( AP1_2_AP0, ColorAP1 );

#if 1
	// \"Glow\" module constants
	const float RRT_GLOW_GAIN = 0.05;
	const float RRT_GLOW_MID = 0.08;

	float saturation = rgb_2_saturation( ColorAP0 );
	float ycIn = rgb_2_yc( ColorAP0 );
	float s = sigmoid_shaper( (saturation - 0.4) / 0.2);
	float addedGlow = 1 + glow_fwd( ycIn, RRT_GLOW_GAIN * s, RRT_GLOW_MID);
	ColorAP0 *= addedGlow;
#endif

#if 1
	// --- Red modifier --- //
	const float RRT_RED_SCALE = 0.82;
	const float RRT_RED_PIVOT = 0.03;
	const float RRT_RED_HUE = 0;
	const float RRT_RED_WIDTH = 135;
	float hue = rgb_2_hue( ColorAP0 );
	float centeredHue = center_hue( hue, RRT_RED_HUE );
	float hueWeight = Square( smoothstep( 0, 1, 1 - abs( 2 * centeredHue / RRT_RED_WIDTH ) ) );
		
	ColorAP0.r += hueWeight * saturation * (RRT_RED_PIVOT - ColorAP0.r) * (1. - RRT_RED_SCALE);
#endif
	
	// Use ACEScg primaries as working space
	float3 WorkingColor = mul( AP0_2_AP1_MAT, ColorAP0 );

	WorkingColor = max( 0, WorkingColor );

	// Pre desaturate
	WorkingColor = lerp( dot( WorkingColor, AP1_RGB2Y ), WorkingColor, 0.96 );
	
	const half ToeScale			= 1 + FilmBlackClip - FilmToe;
	const half ShoulderScale	= 1 + FilmWhiteClip - FilmShoulder;
	
	const float InMatch = 0.18;
	const float OutMatch = 0.18;

	float ToeMatch;
	if( FilmToe > 0.8 )
	{
		// 0.18 will be on straight segment
		ToeMatch = ( 1 - FilmToe  - OutMatch ) / FilmSlope + log10( InMatch );
	}
	else
	{
		// 0.18 will be on toe segment

		// Solve for ToeMatch such that input of InMatch gives output of OutMatch.
		const float bt = ( OutMatch + FilmBlackClip ) / ToeScale - 1;
		ToeMatch = log10( InMatch ) - 0.5 * log( (1+bt)/(1-bt) ) * (ToeScale / FilmSlope);
	}

	float StraightMatch = ( 1 - FilmToe ) / FilmSlope - ToeMatch;
	float ShoulderMatch = FilmShoulder / FilmSlope - StraightMatch;
	
	half3 LogColor = log10( WorkingColor );
	half3 StraightColor = FilmSlope * ( LogColor + StraightMatch );
	
	half3 ToeColor		= (    -FilmBlackClip ) + (2 *      ToeScale) / ( 1 + exp( (-2 * FilmSlope /      ToeScale) * ( LogColor -      ToeMatch ) ) );
	half3 ShoulderColor	= ( 1 + FilmWhiteClip ) - (2 * ShoulderScale) / ( 1 + exp( ( 2 * FilmSlope / ShoulderScale) * ( LogColor - ShoulderMatch ) ) );

	ToeColor		= LogColor <      ToeMatch ?      ToeColor : StraightColor;
	ShoulderColor	= LogColor > ShoulderMatch ? ShoulderColor : StraightColor;

	half3 t = saturate( ( LogColor - ToeMatch ) / ( ShoulderMatch - ToeMatch ) );
	t = ShoulderMatch < ToeMatch ? 1 - t : t;
	t = (3-2*t)*t*t;
	half3 ToneColor = lerp( ToeColor, ShoulderColor, t );

	// Post desaturate
	ToneColor = lerp( dot( float3(ToneColor), AP1_RGB2Y ), ToneColor, 0.93 );

	// Returning positive AP1 values
	return max( 0, ToneColor );
}


//
// Gamut conversion matrices
//
float3x3 OuputGamutMappingMatrix( uint OutputGamut )
{
	// Gamut mapping matrices used later
	const float3x3 AP1_2_sRGB    = mul( XYZ_2_sRGB_MAT, mul( D60_2_D65_CAT, AP1_2_XYZ_MAT ) );
	const float3x3 AP1_2_DCI_D65 = mul( XYZ_2_P3D65_MAT, mul( D60_2_D65_CAT, AP1_2_XYZ_MAT ) );
	const float3x3 AP1_2_Rec2020 = mul( XYZ_2_Rec2020_MAT, mul( D60_2_D65_CAT, AP1_2_XYZ_MAT ) );

	// Set gamut mapping matrix 
	// 0 = sRGB - D65
	// 1 = P3 - D65
	// 2 = Rec.2020 - D65
	// 3 = ACES AP0 - D60
	// 4 = ACES AP1 - D60

	if( OutputGamut == 1 )
		return AP1_2_DCI_D65;
	else if( OutputGamut == 2 )
		return AP1_2_Rec2020;
	else if( OutputGamut == 3 )
		return AP1_2_AP0_MAT;
	else
		return AP1_2_sRGB;
}
#endif