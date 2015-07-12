half2 encode(half3 normal)
{
	float p = sqrt(-normal.z * 8 + 8);
	return normal.xy/p+0.5f;
}

half3 encode(half2 enc_spheremap) {
	half2 enc255 = enc_spheremap * 255;
	half2 residual = floor(frac(enc255) * 16);
	return float3(floor(enc255), residual.x * 16 + residual.y) / 255;
}

half2 decode(half3 enc) {
	half nz = floor(enc.z * 255) / 16;
	return enc.xy + float2(floor(nz) / 16, frac(nz)) / 255;
}

half3 CompressionNormal(half3 n) {
	return encode(encode(n));
}

half3 DeCompressionNormal(half3 n) {
	half2 mrt0 = decode(n);
	half2 fenc = mrt0.xy * 4 - 2;
	half f = dot(fenc, fenc);
	float g = sqrt(1 - f / 4);
	float3 normal;
	normal.xy = fenc*g;
	normal.z = f / 2 - 1;
	return normal;
}