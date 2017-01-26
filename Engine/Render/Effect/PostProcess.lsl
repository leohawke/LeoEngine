(effect
	(shader
		"
		float2 TexCoordFromPos(float4 pos)
		{
			float2 tex = pos.xy / 2;
			#ifdef UV_FLIPPING
			tex.y *= UV_FLIPPING;
			#endif
			tex += 0.5;
			return tex;
		}
		"
	)
)
