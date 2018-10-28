(effect
	(refer material.lsl)
	(shader 
			"
			//https://github.com/ray-cast/ray-mmd/blob/master/Shader/BRDF.fxsub#L293
			float BurleyBRDF(float nl,float nv,float vh,float roughness){
				float energyBias = 0.5*roughness;
				float energyFactor = lerp(1,1/1.51,roughness);

				float Fd90 = energyBias + 2.0*vh*vh*roughness;
				float FdV = lerp(1,Fd90,pow5(1-max(nv,0.1)));
				float FdL = lerp(1,Fd90,pow5(1-nl));
				
				return FdV * FdL * energyFactor;
			}


			float2x3 SurfaceEnergy(Material material,float3 normal,float3 light_source,float3 view_dir,float shadow,float normalizeFactor=1.0)
			{
				float halfway_vec = normalize(view_dir+light_source);

				float nh = saturate(dot(normal,halfway_vec));
				float nl = saturate(dot(normal,light_source));
				float vh = saturate(dot(view_dir,halfway_vec));
				float nv = abs(dot(normal,view_dir)) + 1e-5h;
				float lv = saturate(dot(light_source,view_dir));

				float roughness = material.roughness;
				float occlusion = 1.f;

				float3 diffuse = BurleyBRDF(nl,nv,vh,roughness)*nl*occlusion;

				float3 specular = 0;

				return float2x3(diffuse,specular*nl*occlusion);
			}
			"
	)
)