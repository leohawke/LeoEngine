(effect
	(refer PostProcess.lsl)
	(refer Util.lsl)
	(sampler point_sampler
		(filtering min_mag_mip_point)
		(address_u clamp)
		(address_v clamp)
	)
	(sampler bilinear_sampler
		(filtering min_mag_linear_mip_point)
		(address_u clamp)
		(address_v clamp)
	)
	(cbuffer from_tex
		(float3 src_offset)
		(float3 src_scale)
		(int src_level)
	)
	(texture2D src_2d_tex)
	(shader
				"
				void Blit2DVS(float4 pos : POSITION,
					out float2 oTex : TEXCOORD0,
					out float4 oPos : SV_Position)
				{
					oTex = TexCoordFromPos(pos) * src_scale.xy + src_offset.xy;
					oPos = pos;
				}
				
				float4 Blit2DPS(float2 tc : TEXCOORD0) : SV_Target
				{
						return src_2d_tex.SampleLevel(
					#if LINEAR_SAMPLER
						bilinear_sampler,
					#else
						point_sampler,
					#endif
						tc, src_level);
				}
				"
	)
	(technique (name BlitPoint2D)
		(pass (name p0)
			(macro (name LINEAR_SAMPLER) (value 0))
			(depth_enable false)
			(depth_write_mask false)

			(vertex_shader Blit2DVS)
			(pixel_shader Blit2DPS)
		)
	)
	(technique (name BlitLinear2D) (inherit BlitPoint2D)
		(pass (name p0)
			(macro (name LINEAR_SAMPLER) (value 1))
		)
	)		
)