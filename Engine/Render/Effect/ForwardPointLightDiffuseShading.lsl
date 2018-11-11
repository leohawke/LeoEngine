(effect
	(refer brdf_common.lsl)
	(cbuffer mat
		(float3 albedo)
		(float alpha)
		(float2 metalness)
		(float2 glossiness)
		(float3 specular)
	)
	(cbuffer light
		(float3 view_light_pos)
		(float light_radius)
		(float3 light_color)
		(float light_blubsize)
	)
	(cbuffer obj
		(float4x4 worldviewproj)
		(float4x4 worldview)
		(float4x4 worldviewinvt)
	)
	(texture2D albedo_tex)
	(texture2D glossiness_tex)
	(sampler bilinear_sampler
		(filtering min_mag_linear_mip_point)
		(address_u clamp)
		(address_v clamp)
	)
	(shader 
		"
		float AttenuationTerm(float3 light_source,float radius,float blubsize){
				float invRadius = 1/radius;
				float d = length(light_source);
				float fadeoutFactor = saturate((radius-d)*(invRadius/0.2h));
				d = max(d-blubsize,0);
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
						in float2 TexCoord:TEXCOORD,
					out float4 ClipPos:SV_POSITION,
					out float2 Tex:TEXCOORD0,
					out float4 ViewPos:TEXCOORD1,
					out float3 ViewNormal:TEXCOORD2
		)
		{
			ClipPos = mul(float4(Postion,1.0f),worldviewproj);
			ViewPos = mul(float4(Postion,1.0f),worldview);
			ViewNormal = mul(float4(Normal,0.0f),worldviewinvt).xyz;
			Tex = TexCoord;
		}

		void PointLightPS(in float4 ClipPos:SV_POSITION,
			in float2 tex:TEXCOORD0,
			in float3 view_postion:TEXCOORD1,
			in float3 view_normal:TEXCOORD2,
			out float4 color :SV_Target
		)
		{
			float3 view_dir = -normalize(view_postion);
			float shadow = 1;

			Material material;
			material.normal = normalize(view_normal);
			material.albedo = albedo*albedo_tex.Sample(bilinear_sampler,tex).rgb;
			material.metalness = metalness.x;
			material.specular = specular;
			material.alpha = alpha;
			material.roughness = glossiness.y > 0.5?glossiness.x*glossiness_tex.Sample(bilinear_sampler,tex).r:glossiness.x;

			float3 diffuse,specular = 0;
			ShadingMaterial(material,view_postion,view_dir,shadow,diffuse,specular);

			color.xyz = diffuse*material.albedo + specular;
			color.w = 1.0f;
		}
		"
	)
	(technique (name PointLight)
		(pass (name p0)
			(vertex_shader ForawdVS)
			(pixel_shader PointLightPS)
		)
	)
)