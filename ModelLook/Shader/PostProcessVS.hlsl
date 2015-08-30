struct Vertex {
	float4 PosH : POSITION;
	float2 Tex :TEXCOORD;
};

struct Pixel {
	float4 PosH : SV_POSITION;
	float2 Tex :TEXCOORD;
};

Pixel main(Vertex vin)
{
	Pixel o;
	o.PosH = vin.PosH;
	o.Tex = vin.Tex;
	return o;
}