(effect
	(include Common.h)
	(shader
	"
		float4x4 ScreenToShadowMatrix;
		// .x:DepthBias, .y:SlopeDepthBias, .z:ReceiverBias, .w: MaxSubjectZ - MinSubjectZ
		float4 ProjectionDepthBiasParameters;
		float FadePlaneOffset;
		float InvFadePlaneLength;
		Texture2D ShadowDepthTexture;
		sampler ShadowDepthTextureSampler;

		//View
		float4 BufferSizeAndInvSize;
		Texture2D SceneDepthTexture;
		sampler SceneDepthTextureSampler;
		float4x4 ScreenToWorld;

		float CalcSceneDepth(float2 ScreenUV)
		{
			return ConvertFromDeviceZ(SceneDepthTexture.SampleLevel(SceneDepthTextureSampler,ScreenUV,0).r);
		}

		void Main(in float4 SVPos:SV_POSITION,
		out float4 OutColor : SV_Target0
		)
		{
			float2 ScreenUV = SVPos.xy * BufferSizeAndInvSize;
			float SceneW =  CalcSceneDepth(ScreenUV);

			float4 ScreenPosition = float4(ScreenUV*SceneW,SceneW,1);
			float4 ShadowPosition = mul(ScreenPosition, ScreenToShadowMatrix);
			float3 WorldPosition = mul(ScreenPosition, ScreenToWorld).xyz;

			float ShadowZ = ShadowPosition.z;
			ShadowPosition.xyz /= ShadowPosition.w;

			float Shadow = 1;

			float BlendFactor = 1;

			float LightSpacePixelDepthForOpaque = min(ShadowZ, 0.99999f);

			Shadow = LightSpacePixelDepthForOpaque < ShadowDepthTexture.SampleLevel(ShadowDepthTextureSampler, ShadowPosition.xy, 0).r;

			BlendFactor = 1.0f - saturate((SceneW - FadePlaneOffset) * InvFadePlaneLength);

			OutColor.a = BlendFactor;

			// Apply a 1/2 power to the input, which allocates more bits for the darks and prevents banding
			OutColor.rgb = sqrt(Shadow);
		}
	")
)