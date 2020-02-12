(effect
	(shader 
		"
			float luminance(float3 rgb){
				return dot(rgb,float3(0.2126f,0.7152f,0.0722f));
			}

			float pow5(float x) {float xx = x*x;return xx*xx*x;}

			float3 transform_quat(float3 v,float4 quat)
			{
				return v + cross(quat.xyz, cross(quat.xyz, v) + quat.w*v) * 2;
			}

			float3 restore_normal(float2 normal_xy)
			{
				float3 normal;
				normal.xy = normal_xy;
				normal.z = sqrt(max(0.0f, 1 - dot(normal.xy, normal.xy)));
				return normalize(normal);
			}

			float3 decompress_normal(float4 comp_normal)
			{
				return restore_normal(comp_normal.rg * 2 - 1);
			}
		"
	)
)