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
    "
    )
)