(effect
	(refer material.lsl)
	(shader 
			"
			#define POINT_LIGHT 0
			#define SPOT_LIGHT 1
			#define DIRECTIONAL_LIGHT 2
			struct DirectLight{
				//direction for spot and directional lights (View space).
				float3 direction;
				//range for point and spot lights(Maximum distance of influence)
				float range;
				//position for spot and direction lights(View Space)
				float3 position;
				//outer angle for spot light(radian)
				float outerangle;
				//blub size for point light or inneragnle for spot light
				float blub_innerangle;

				//The color of emitted light(linear RGB color)
				float3 color;
				//
				uint type;
			};

			struct Light{
				float3 color;
				float3 direction;
				float attenuation;
				float nol;
			};

			float DistanceAttenuationTerm(float3 light_source,float radius,float blubsize){
				float invRadius = 1/radius;
				float d = length(light_source);
				float fadeoutFactor = saturate((radius-d)*(invRadius/0.2h));
				d = max(d-blubsize,0);
				float dnom = 1+ d/blubsize;
				float attenuation = fadeoutFactor * fadeoutFactor/(dnom*dnom);
				return attenuation;
			}

			Light GetPointLight(in DirectLight light_params,in Material material){
				Light light;

				float3 light_source = light_params.position - material.position;
				float3 light_source_normal = normalize(light_source);

				light.attenuation = DistanceAttenuationTerm(light_source,light_params.range,light_params.blub_innerangle);

				light.direction = light_source_normal;

				light.nol = saturate(dot(material.normal,light_source_normal));

				light.color = light_params.color;

				return light;
			}
			"
	)
)