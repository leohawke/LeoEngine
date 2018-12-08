(effect
	(refer Util.lsl)
	(shader 
			"
			struct Material{
				float3 normal;
				float alpha;

				float3 albedo;
				float metalness;

				float3 diffuse;

				float roughness;

				float3 position;
			};

			float SmoothnessToRoughness(float smoothness){
				return (1.0f-smoothness)*(1.0f-smoothness);
			}
			"
	)
)