(effect
	(refer PostProcess.lsl)
	(refer Util.lsl)
	(parameter (type sampler) (name point_sampler)
		(filtering min_mag_mip_point)
		(address_u clamp)
		(address_v clamp)
	)
	(parameter (type sampler) (name bilinear_sampler)
		(filtering min_mag_linear_mip_point)
		(address_u clamp)
		(address_v clamp)
	)
	(cbuffer (name from_tex)
		(parameter (type float3) (name src_offset))
		(parameter (type float3) (name src_scale))
		(parameter (type int) (name src_level))
	)
	(parameter (type texture2D) (name src_2d_tex))
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
	(technique (name PointCopy)
		(pass (name p0)
			(macro (name LINEAR_SAMPLER) (value 0))
			(depth_enable false)
			(depth_write_mask false)

			(vertex_shader Blit2DVS)
			(pixel_shader Blit2DPS)
		)
	)
	(technique (name BilinearCopy) (inherit PointCopy)
		(pass (name p0)
			(macro (name LINEAR_SAMPLER) (value 1))
		)
	)		
)