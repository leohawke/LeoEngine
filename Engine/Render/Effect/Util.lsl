(effect
	(shader 
		"
			float luminance(float3 rgb){
				return dot(rgb,float3(0.2126f,0.7152f,0.0722f));
			}

			float pow5(float x) {float xx = x*x;return xx*xx*x;}
		"
	)
)