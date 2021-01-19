(effect
    (include Common.h)
    (refer PostProcess/TonemapCommon.lsl)
    (Texture2D ColorTexture)
    (Texture3D ColorGradingLUT)
	(sampler ColorSampler)
	(sampler ColorGradingLUTSampler)
    (shader
"
#define TONEMAPPER_OUTPUT_sRGB 0
#define TONEMAPPER_OUTPUT_Rec709 1
#define TONEMAPPER_OUTPUT_ExplicitGammaMapping 2
#define TONEMAPPER_OUTPUT_ACES1000nitST2084 3
#define TONEMAPPER_OUTPUT_ACES2000nitST2084 4
#define TONEMAPPER_OUTPUT_ACES1000nitScRGB 5
#define TONEMAPPER_OUTPUT_ACES2000nitScRGB 6
#define TONEMAPPER_OUTPUT_LinearEXR 7
#define TONEMAPPER_OUTPUT_NoToneCurve 8
#define TONEMAPPER_OUTPUT_WithToneCurve 9

#ifndef DIM_OUTPUT_DEVICE
	#define DIM_OUTPUT_DEVICE (TONEMAPPER_OUTPUT_sRGB)
#endif

static const float LUTSize = 32;

half3 ColorLookupTable( half3 LinearColor )
{
	float3 LUTEncodedColor;
	// Encode as ST-2084 (Dolby PQ) values
	#if (DIM_OUTPUT_DEVICE == TONEMAPPER_OUTPUT_ACES1000nitST2084 || DIM_OUTPUT_DEVICE == TONEMAPPER_OUTPUT_ACES2000nitST2084 || DIM_OUTPUT_DEVICE == TONEMAPPER_OUTPUT_ACES1000nitScRGB || DIM_OUTPUT_DEVICE == TONEMAPPER_OUTPUT_ACES2000nitScRGB || DIM_OUTPUT_DEVICE == TONEMAPPER_OUTPUT_LinearEXR || DIM_OUTPUT_DEVICE == TONEMAPPER_OUTPUT_NoToneCurve || DIM_OUTPUT_DEVICE == TONEMAPPER_OUTPUT_WithToneCurve)
		// ST2084 expects to receive linear values 0-10000 in nits.
		// So the linear value must be multiplied by a scale factor to convert to nits.
		LUTEncodedColor = LinearToST2084(LinearColor * LinearToNitsScale);
	#else
		LUTEncodedColor = LinToLog( LinearColor + LogToLin( 0 ) );

	#endif

	float3 UVW = LUTEncodedColor * ((LUTSize - 1) / LUTSize) + (0.5f / LUTSize);

	half3 OutDeviceColor = ColorGradingLUT.Sample(ColorGradingLUTSampler, UVW ).rgb;

	return OutDeviceColor * 1.05;
}

void MainVS(
	in 					float4 InPosition 				: POSITION,
	in 					float2 InTexCoord 				: TEXCOORD0,
	out noperspective 	float2 OutTexCoord 				: TEXCOORD0,
	out 				float4 OutPosition 				: SV_POSITION
	)
{
	DrawRectangle(InPosition,InTexCoord,OutPosition,OutTexCoord);
}

float4 SampleSceneColor(float2 SceneUV)
{
	return ColorTexture.Sample(ColorSampler, SceneUV);
}

float4 TonemapCommonPS(
	float2 UV
)
{
	float4 OutColor = 0;

	float2 SceneUV = UV.xy;

	half4 SceneColor = SampleSceneColor(SceneUV);

	half3 LinearColor = SceneColor.rgb;

	half3 OutDeviceColor = ColorLookupTable( LinearColor );

	half LuminanceForPostProcessAA  = dot(OutDeviceColor, half3(0.299f, 0.587f, 0.114f));

	// RETURN_COLOR not needed unless writing to SceneColor
	OutColor = float4(OutDeviceColor, saturate(LuminanceForPostProcessAA));

	return OutColor;
}

void MainPS(
	in noperspective float2 UV : TEXCOORD0,
	out float4 OutColor : SV_Target0)
{
	OutColor = TonemapCommonPS(UV);
}
"
    )
)