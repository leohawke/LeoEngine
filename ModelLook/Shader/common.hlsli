float3 Sobel(float2 texcoord, Texture2D tex, SamplerState s) {
	float4 r = tex.GatherRed(s, texcoord,
		int2(-1, -1),//top-left00
		int2(0, -1),//top-middle10
		int2(1, -1),//top-right20
		int2(-1, 0),//middle-left01
		);
	float4 g = tex.GatherRed(s, texcoord,
		int2(1, 0),//middle-right21
		int2(-1, 1),//top-middle02
		int2(0, 1),//top-right12
		int2(1, 1),//middle-left22
		);

	//[ r.x r.y r.z ]   [ 1  2  1 ]
	//[ r.w  0  g.x ] * [ 0  0  0 ] = dx
	//[ g.y g.z g.w ]   [-1 -2 -1 ]
	//http://en.wikipedia.org/wiki/Sobel_operator
	float dx = r.x - r.z + 2.f*r.w - 2.f*g.x + g.y - g.w;
	//[ r.x r.y r.z ]   [ 1  0  -1 ]
	//[ r.w  0  g.x ] * [ 2  0  -2 ] = dy
	//[ g.y g.z g.w ]   [ 1 -0  -1 ]
	float dy = r.x + 2.f*r.y + r.z - g.y - 2.f*g.z - g.w;

	float dz = 0.01f*sqrt(max(0.f, 1.f - dx*dx - dy*dy));

	return normalize(float3(2.f*dx, dz, 2.f*dy));
}