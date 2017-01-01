(effect
	(shader
		"
		float2 TexCoordFromPos(float4 pos)
		{
			float2 tex = pos.xy / 2;
			tex.y *= UV_FLIPPING;
			tex += 0.5;
			return tex;
		}
		"
	)
)
