cbuffer perLight :register(b0) {
	matrix WorldView;
	matrix Proj;
}

void DeferredRenderingDepthOnlyVS(float3 pos : POSITION,
	out float4 oPos : SV_Position)
{
	oPos = mul(float4(pos,1.f),mul(WorldView,Proj));
}