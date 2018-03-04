(effect
	(refer PostProcess.lsl)
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
	(texture2D src_tex)
	(shader
				"
				float4 CopyPS(float2 tc0 : TEXCOORD0) : SV_Target
				{
					return src_tex.Sample(point_sampler, tc0);
				}
				
				float4 BilinearCopyPS(float2 tc0 : TEXCOORD0) : SV_Target
				{
					return src_tex.Sample(bilinear_sampler, tc0);
				}
				"
	)
	(technique (name PointCopy)
		(pass (name p0)
			(depth_enable false)
			(depth_write_mask false)

			(vertex_shader PostProcessVS)
			(pixel_shader CopyPS)
		)
	)
	(technique (name BilinearCopy) (inherit PointCopy)
		(pass (name p0)
			(pixel_shader BilinearCopyPS)
		)
	)		
)