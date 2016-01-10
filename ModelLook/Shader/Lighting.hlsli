//Dbp = (a+2) / 2PI (n.m)@
//Blinn-Phong (n.h)@
float roughness_term(float3 halfway_vec, float3 normal, float roughness)
{
	return pow(max(dot(halfway_vec, normal), 0.0f), roughness);
}

//todo :add light params :attn
float attenuation_term(float3 light_pos, float3 pos, float3 atten)
{
	float3 v = light_pos - pos;
	float d2 = dot(v, v);
	float d = sqrt(d2);
	return 1 / dot(atten, float3(1, d, d2));
}

float spot_lighting(float3 light_pos, float3 light_dir, float2 cos_cone, float3 pos)
{
	// cos_cone is (cos_outer_cone, cos_inner_cone)

	float3 v = normalize(pos - light_pos);
	float cos_direction = dot(v, light_dir);

	return smoothstep(cos_cone.x, cos_cone.y, cos_direction);
}

float4 CalcColor(float lambert,float spec,float atten,float3 light_color) {
	float2 ds = lambert*atten*float2(1, spec);
	return ds.xxxy*float4(light_color, 1.f);
}

float4 CalcDRLighting(float3 light_pos, float3 pos_es, float3 normal, float3 view_dir,
	float shininess, float atten, float3 light_color,float range) {
	float4 lighting = 0;
	float3 dir = light_pos - pos_es;
	float dist = length(dir);
	if (dist < range) {
		dir /= dist;
		float lambert = dot(normal, dir);
		if (lambert > 0) {
			float spec = roughness_term(normalize(dir - view_dir), normal, shininess);
			lighting = CalcColor(lambert, spec, atten, light_color);
		}
	}
	return lighting;
}