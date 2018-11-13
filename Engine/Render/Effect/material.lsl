(effect
	(refer Util.lsl)
	(shader 
			"
			struct Material{
				float3 normal;
				float3 albedo;
				float metalness;

				float alpha;
				float roughness;
			};

			float SmoothnessToRoughness(float smoothness){
				return (1.0f-smoothness)*(1.0f-smoothness);
			}
			"
	)
)