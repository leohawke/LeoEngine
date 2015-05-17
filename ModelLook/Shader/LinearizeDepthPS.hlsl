Texture2D texDepth :register(t0);

SamplerState samPoint : register(s0);



cbuffer ClipPlane :register(c0) {
	//Q = far/(far-near);
	//Mul = near*Q;
	float4 MulQ;
}

struct VertexOut
{
	float4 PosH : SV_POSITION;
	float3 ToFarPlane :TEXCOORD0;
	float2 Tex : TEXCOORD1;
};

float main(VertexOut pin) : SV_TARGET
{
	float nolineardepth = texDepth.Sample(samPoint,pin.Tex).r;

return MulQ.x / (MulQ.y - nolineardepth);
}