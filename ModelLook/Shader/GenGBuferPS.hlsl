SamplerState samAniso:register(s0);

Texture2D texDiffuse:register(t0);

struct Material
{
	float3 Specular; // w = SpecPower
	float SpecularPow;
};

cbuffer ObjectMaterial :register(c0) {
	Material Object;
}

struct VertexOut
{
	float4 PosH     : SV_POSITION;
	half3 NormalV  : NORMAL;
	float2 Tex      : TEXCOORD;
};

half2 encode(half3 normal)
{
	return normalize(normal.xy) * sqrt(normal.z * 0.5 + 0.5);
}
half3 decode(half2 n)
{
	float3 normal;
	normal.z = dot(n, n) * 2 - 1;
	normal.xy = normalize(n) * sqrt(1 - normal.z * normal.z);
	return normal;
}

half3 encode(half2 enc_spheremap) {
	half2 enc255 = enc_spheremap * 255;
	half2 residual = floor(frac(enc255) * 16);
	return float3(floor(enc255), residual.x * 16 + residual.y) / 255;
}

half2 decode(half3 enc) {
	half nz = floor(enc.z * 255) / 16;
	return enc.xy + float2(floor(nz) / 16, frac(nz)) / 255;
}

void GBufferMRTPS(VertexOut pin,
	out half4 NormalSpecPow: SV_TARGET0,
	out half4 DiffuseSpec : SV_TARGET1)
{
	NormalSpecPow.xyz = encode(encode(pin.NormalV));
	NormalSpecPow.w = Object.SpecularPow;

	DiffuseSpec.rgb = texDiffuse.Sample(samAniso, pin.Tex).rgb;
	DiffuseSpec.a = dot(Object.Specular,half3(0.2126f, 0.7152f, 0.0722f));
}