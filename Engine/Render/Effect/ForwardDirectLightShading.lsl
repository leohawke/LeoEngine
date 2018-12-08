(effect
	(refer brdf_common.lsl)
	(refer DirectLight.lsl)
	(cbuffer mat
		(float3 albedo)
		(float alpha)
		(float2 metalness)
		(float2 glossiness)
	)
	(cbuffer global
		(uint light_count)
	)
	(StructuredBuffer (elemtype DirectLight) lights)
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
		void SurfaceShading(in Light light,in Material material,float3 view_dir,inout float3 diffuse,inout float3 specular){
			float2x3 energy = SurfaceEnergy(material,light.direction,view_dir);

			diffuse = energy[0]*light.attenuation*light.nol*light.color;
			specular = energy[1]*light.attenuation*light.nol*light.color;
		}

		struct LightingResult
		{
			float3 diffuse;
			float3 specular;
		};

		void ShadingMaterial(Material material,float3 view_dir,float shadow,float occlusion,inout float3 diffuse,inout float3 specular)
		{
			LightingResult total = (LightingResult)0;
			for(int i = 0; i!=light_count;++i ){
				DirectLight direct_light = lights[i];
				Light light = GetPointLight(direct_light,material);

				LightingResult result = (LightingResult)0;
				SurfaceShading(light,material,view_dir,result.diffuse,result.specular);

				total.diffuse += result.diffuse;
				total.specular += result.specular;
			}
			diffuse = total.diffuse;
			specular = toatal.specular;
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
					out float3 ViewPos:TEXCOORD1,
					out float3 ViewNormal:TEXCOORD2
		)
		{
			ClipPos = mul(float4(Postion,1.0f),worldviewproj);
			ViewPos = mul(float4(Postion,1.0f),worldview).xyz;
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
			float occlusion = 1;

			Material material;
			material.normal = normalize(view_normal);
			material.diffuse = material.albedo*(1-material.metalness);
			material.albedo = albedo*albedo_tex.Sample(bilinear_sampler,tex).rgb;
			material.metalness = metalness.x;
			material.alpha = alpha;
			material.roughness = glossiness.y > 0.5?glossiness.x*glossiness_tex.Sample(bilinear_sampler,tex).r:glossiness.x;
			material.position = view_position;

			float3 diffuse,specular = 0;
			ShadingMaterial(material,view_dir,shadow,occlusion,diffuse,specular);

			color.xyz = diffuse + specular;
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