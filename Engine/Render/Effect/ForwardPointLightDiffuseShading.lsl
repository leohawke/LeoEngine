(effect
	(refer brdf_common.lsl)
	(cbuffer (name mat)
		(parameter (type float3 albedo))
		(parameter (type float metalness))
		(parameter (type float3 specular))
		(parameter (type float alpha))
		(parameter (type float smoothness))
	)
	(cbuffer (name light)
		(parameter (type float3 view_light_pos))
		(parameter (type float light_radius))
		(parameter (type float3 light_color))
		(parameter (type float light_blubsize))
	)
	(cbuffer (name obj)
		(parameter (type float4x4 worldviewporj))
		(parameter (type float4x4 worldview))
	)
	(shader 
		"
		float AttenuationTerm(float3 light_source,float radius,float blubsize){
				const float invRadius = 1/raidus;
				float d = length(light_source);
				float fadeoutFactor = saturate((radius-d)*(invRadius/0.2h));
				d = max(d-blubSize,0);
				float dnom = 1+ d/blubsize;
				float attenuation = fadeoutFactor * fadeoutFactor/(dnom*dnom);
				return attenuation;
		}

		void ShadingMaterial(Material material,float3 view_position,float3 view_dir,float shadow,inout float3 diffuse,inout float3 specular)
		{
			float3 light_source = view_light_pos - view_position;
			float3 light_source_normal = normalize(light_source);

			float3 atten = AttenuationTerm(light_source,light_radius,light_blubsize);
			atten *= light_color;

			float2x3 energy = SurfaceEnergy(material,material.normal,light_source_normal,view_dir,shadow);
			diffuse = energy[0] /*+ Subsurface*/;
			diffuse *= atten;

			specular = energy[1];
			specular *= atten;
		}
		"
	)
	(shader
		"
		void ForawdVS(in float3 Postion:POSITION,
						in float3 Normal:NORMAL,
					out float4 ClipPos:SV_POSITION,
					out float4 ViewPos:TEXCOORD1
		)
		{
			ClipPos = mul(float4(Postion,1.0f),worldviewporj);
			ViewPos = mul(float4(Postion,1.0f),worldview);
		}

		void PointLightPS(in float4 ClipPos:SV_POSITION,
			in float3 view_postion:TEXCOORD1,
			out float4 color :SV_Target
		)
		{
			float3 view_dir = normalize(view_postion);
			float shadow = 1;

			float3 diffuse,specular = 0;
			ShadingMaterial(material,view_postion,view_dir,shadow,diffuse,specular);

			color.xyz = diffuse + specular;
			color.w = 1.0f;
		}
		"
	)
	(technique (name PointLight)
		(pass (name p0)
			(depth_enable false)
			(depth_write_mask false)
			(vertex_shader ForawdVS)
			(pixel_shader PointLightPS)
		)
	)
)