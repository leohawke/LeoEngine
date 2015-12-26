cbuffer ui_vsparams : register(b0) {
	float2 inv_xy;
};

void main( float4 pos : POSITION,out float2 Tex:TEXCOORD0, out float4 PosH : SV_POSITION)
{
	PosH.xy = pos.xy* inv_xy;
	
	PosH.x = PosH.x *2  - 1;
	PosH.y = 1 -  2* PosH.y;
	PosH.z = 1.f;
	PosH.w = 1.f;

	Tex = pos.zw;
}