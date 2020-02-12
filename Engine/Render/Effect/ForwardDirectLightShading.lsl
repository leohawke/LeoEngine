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
		(float4x4 world)
	)
	(cbuffer camera
		(float3 camera_pos)
		(float4x4 viewproj)
	)
	(texture2D albedo_tex)
	(texture2D glossiness_tex)
	(texture2D normal_tex)
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
			for(uint i = 0; i!=light_count;++i ){
				DirectLight direct_light = lights[i];

				Light light = GetLight(direct_light,material);

				LightingResult result = (LightingResult)0;
				SurfaceShading(light,material,view_dir,result.diffuse,result.specular);

				total.diffuse += result.diffuse;
				total.specular += result.specular;
			}
			diffuse = total.diffuse;
			specular = total.specular;
		}
		"
	)
	(shader
		"
		void ForwardVS(in float3 Postion:POSITION,
						in float4 Tangent_Quat:TANGENT,
						in float2 TexCoord:TEXCOORD,
					out float4 ClipPos:SV_POSITION,
					out float2 Tex:TEXCOORD0,
					out float3 WorldPos:TEXCOORD1,
					out float3 ViewDir:TEXCOORD2,
					out float3 oTsToW0:TEXCOORD3,
					out float3 oTsToW1:TEXCOORD4,
					out float3 oTsToW2:TEXCOORD5
		)
		{
			WorldPos = mul(float4(Postion,1.0f),world);

			ClipPos = mul(float4(WorldPos,1.0f),viewproj);
			Tex = TexCoord;
			ViewDir = camera_pos - WorldPos;

			Tangent_Quat = Tangent_Quat * 2 - 1;

			float3x3 obj_to_ts;
			obj_to_ts[0] = transform_quat(float3(1, 0, 0), Tangent_Quat);
			obj_to_ts[1] = transform_quat(float3(0, 1, 0), Tangent_Quat) * sign(Tangent_Quat.w);
			obj_to_ts[2] = transform_quat(float3(0, 0, 1), Tangent_Quat);

			float3x3 ts_to_world = mul(obj_to_ts,(float3x3)world);

			oTsToW0 = ts_to_world[0];
			oTsToW1 = ts_to_world[1];
			oTsToW2 = ts_to_world[2];
		}

		void ForwardLightPS(in float4 ClipPos:SV_POSITION,
			in float2 tex:TEXCOORD0,
			in float3 world_pos:TEXCOORD1,
			in float3 view_dir :TEXCOORD2,
			in float3 t:TEXCOORD3,
			in float3 b:TEXCOORD4,
			in float3 n:TEXCOORD5,
			out float4 color :SV_Target
		)
		{
			float3 world_normal = normalize(n);

			float3x3 ts_to_world;
			ts_to_world[0] = normalize(t);
			ts_to_world[1] = normalize(b);
			ts_to_world[2] = world_normal;

			float3 normal =decompress_normal(normal_tex.Sample(bilinear_sampler,tex));

			world_normal = normalize(mul(normal,ts_to_world));

			view_dir = normalize(view_dir);
			float shadow = 1;
			float occlusion = 1;

			Material material;
			material.normal = world_normal;
			material.albedo = albedo*albedo_tex.Sample(bilinear_sampler,tex).rgb;
			material.metalness = metalness.x;
			material.alpha = alpha;
			material.roughness = glossiness.y > 0.5?glossiness.x*glossiness_tex.Sample(bilinear_sampler,tex).r:glossiness.x;
			material.position = world_pos;
			material.diffuse = material.albedo*(1-material.metalness);

			float3 diffuse,specular = 0;
			ShadingMaterial(material,view_dir,shadow,occlusion,diffuse,specular);

			color.xyz = diffuse + specular;
			color.w = 1.0f;
		}
		"
	)
	(technique (name PointLight)
		(pass (name p0)
			(vertex_shader ForwardVS)
			(pixel_shader ForwardLightPS)
		)
	)
)