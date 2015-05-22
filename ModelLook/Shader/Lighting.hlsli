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