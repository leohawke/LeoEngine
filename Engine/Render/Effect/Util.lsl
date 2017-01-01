(effect
	(shader
		"
		float4 Tex2DSampleLevel(Texture2D tex, sampler sa, float2 tc, float mip, float mip_bias)
		{
			#if defined(PIXEL_SHADER) && NO_TEX_LOD
				return tex.SampleBias(sa, tc, mip + mip_bias + 0.5f);
			#else
				return tex.SampleLevel(sa, tc, mip);
			#endif
		}
		"
	)
)