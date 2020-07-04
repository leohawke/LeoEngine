(effect
    (shader
    "
const static float PI = 3.1415926535897932f;

float Square( float x )
{
	return x*x;
}

float2 Square( float2 x )
{
	return x*x;
}

float3 Square( float3 x )
{
	return x*x;
}

float4 Square( float4 x )
{
	return x*x;
}

/** Output of the screen vertex shader. */
struct ScreenVertexOutput
{
	noperspective float2 UV : TEXCOORD0;
	float4 Position : SV_POSITION;
};

struct WriteToSliceGeometryOutput
{
    ScreenVertexOutput Vertex;
    uint LayerIndex : SV_RenderTargetArrayIndex;
};

/** Used for calculating vertex positions and UVs when drawing with DrawRectangle */
void DrawRectangle(
	in float4 InPosition,
	in float2 InTexCoord,
	out float4 OutPosition,
	out float2 OutTexCoord)
{
	OutPosition = InPosition;
	OutPosition.xy = -1.0f + 2.0f * InPosition.xy;
	OutPosition.xy *= float2( 1, -1 );
	OutTexCoord.xy = InTexCoord.xy ;
}

    "
    )
)