Texture2D texNormals:register(t0);
SamplerState normalSampler:register(s0);

Texture2D texDiffuse:register(t1);
SamplerState anisoSampler:register(s1);


void CompressUnsignedNormalToNormalsBuffer(inout half3 vNormal)
{
	// renormalize (needed if any blending or interpolation happened before)
	vNormal.rgb = normalize(vNormal.rgb);
	// get unsigned normal for cubemap lookup (note the full float precision is required)
	half3 vNormalUns = abs(vNormal.rgb);
	// get the main axis for cubemap lookup
	half maxNAbs = max(vNormalUns.z, max(vNormalUns.x, vNormalUns.y));
	// get texture coordinates in a collapsed cubemap
	float2 vTexCoord = vNormalUns.z<maxNAbs ? (vNormalUns.y<maxNAbs ? vNormalUns.yz : vNormalUns.xz) : vNormalUns.xy;
	vTexCoord = vTexCoord.x < vTexCoord.y ? vTexCoord.yx : vTexCoord.xy;
	vTexCoord.y /= vTexCoord.x;
	// fit normal into the edge of unit cube
	vNormal.rgb /= maxNAbs;
	// look-up fitting length and scale the normal to get the best fit
	float fFittingScale = texNormals.Sample(normalSampler,vTexCoord).a;
	// scale the normal to get the best fit
	vNormal.rgb *= fFittingScale;
	// squeeze back to unsigned
	vNormal.rgb = vNormal.rgb * .5h + .5h;
}


struct VertexOut
{
	float4 PosH     : SV_POSITION;
	half3 NormalV  : NORMAL;
	float2 Tex      : TEXCOORD;
	float3 PosV     : POSITION;
};

void GBufferMRTPS(VertexOut pin,
	out float4 NormalDepth: SV_TARGET0,
	out float4 DiffuseSpec: SV_TARGET1)
{
	half3 restoreNormal = pin.NormalV;
	CompressUnsignedNormalToNormalsBuffer(restoreNormal);
	NormalDepth.rgb = restoreNormal;
	NormalDepth.a = pin.PosV.z;

	float4 diffuse = texDiffuse.Sample(anisoSampler, pin.Tex);
	clip(diffuse.a-0.1f);
	DiffuseSpec.rgb = diffuse.rgb;
	DiffuseSpec.a = 1.f;
}