Texture2D<float4> texLighting:register(t0);
Texture2D<float4> texDiffuseAlpha:register(t1);
Texture2D<half> texAmbient:register(t2);

float4 main() : SV_TARGET
{
	(texLighting.rgb*texDiffuseAlpha.rgb + (texDiffuseAlpha.a + 2) / 8 * texLighting.a)*ao;
}