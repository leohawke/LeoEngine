(effect
	(refer material.lsl)
	(shader 
			"
			float D_GGX(float nh,float roughness){
				float a = nh * roughness;
				float k = roughness / (1-nh*nh+a*a);
				return k*k*(1/3.141516926f);
			}

			float V_SmithGGXCorrelated(float nv,float nl,float roughness){
				float a2 = roughness*roughness;
				float GGXV = nl * sqrt(nv*nv*(1-a2)+a2);
				float GGXL = nv * sqrt(nl*nl*(1-a2)+a2);
				return 0.5/(GGXV+GGXL);
			}

			float3 F_Schlick(float vh,float3 f0,float f90){
				return f0+(f90-f0)*pow(1-vh,5);
			}

			//https://github.com/ray-cast/ray-mmd/blob/master/Shader/BRDF.fxsub#L293
			//float energyBias = 0.5*roughness;
			//float Fd90 = energyBias + 2.0*vh*vh*roughness;
			float BurleyBRDF(float nl,float nv,float vh,float roughness){
				float energyBias = 0.5*roughness;
				float energyFactor = lerp(1,1/1.51,roughness);

				float Fd90 = energyBias + 2.0*vh*vh*roughness;
				float FdV = lerp(1,Fd90,pow5(1-max(nv,0.1)));
				float FdL = lerp(1,Fd90,pow5(1-nl));
				
				return FdV * FdL * energyFactor;
			}

			//f90
			float SpecularMicroOcclusion(float3 f0){
				return saturate(dot(f0,0.33333)*50);
			}

			float3 SpecularBRDF_GGX(float nh,float nl,float vh,float nv,float roughness,float3 albedo,float metallic)
			{
				float d = D_GGX(nh,roughness);
				float v = V_SmithGGXCorrelated(nv,nl,roughness);

				float3 f0 = lerp(0.04,albedo,metallic);
				float f90 = SpecularMicroOcclusion(f0);

				float3 f = F_Schlick(vh,f0,f90);

				return d*v*f;
			}


			float2x3 SurfaceEnergy(Material material,float3 light_source,float3 view_dir)
			{
				float3 normal = material.normal;
				float3 halfway_vec = normalize(view_dir+light_source);

				float nh = saturate(dot(normal,halfway_vec));
				float nl = saturate(dot(normal,light_source));
				float vh = saturate(dot(view_dir,halfway_vec));
				float nv = abs(dot(normal,view_dir)) + 1e-5h;
				float lv = saturate(dot(light_source,view_dir));

				float roughness = material.roughness;

				float3 diffuse = BurleyBRDF(nl,nv,vh,roughness);

				float3 specular = SpecularBRDF_GGX(nh,nl,vh,nv,roughness,material.albedo,material.metalness);

				return float2x3(diffuse*material.diffuse,specular);
			}
			"
	)
)