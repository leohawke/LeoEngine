(effect
	(cbuffer obj
		(float4x4 world)
	)
	(cbuffer camera
		(float3 camera_pos)
		(float4x4 viewproj)
	)
	(texture2D normal_tex)
	(shader
		"
		void ForwardVS(in float3 Postion:POSITION,
						in float4 Tangent_Quat:TANGENT,
						in float2 TexCoord:TEXCOORD,
					out float4 ClipPos:SV_POSITION,
					out float3 oTsToW0:TEXCOORD3,
					out float3 oTsToW1:TEXCOORD4,
					out float3 oTsToW2:TEXCOORD5
		)
		{
			float3 WorldPos = mul(float4(Postion,1.0f),world).xyz;

			ClipPos = mul(float4(WorldPos,1.0f),viewproj);

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

		void ForwardPS(in float4 ClipPos:SV_POSITION,
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

			color =float4(world_normal.xyz,1);
		}
		"
	)
	(technique (name PreZ)
		(pass (name p0)
			(vertex_shader ForwardVS)
			(pixel_shader ForwardPS)
		)
	)
)