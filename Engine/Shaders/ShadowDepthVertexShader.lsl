(effect
	(refer Util.lsl)
	(shader
	"
		cbuffer ShadowDepthPass
		{
			float4x4 ViewMatrix;
			float4x4 ProjectionMatrix;
			//bias,slopebias,maxslopebias,1/maxdepth
			float4 ShadowParams;
		};

		void CalcShadowDepthOutput(
			float4x4 WorldToClipMatrix, 
			float4x4 WorldToShadowMatrix, 
			float4 WorldPosition, 
			float3 WorldVertexNormal, 
			out float4 OutPosition, 
			out float ShadowDepth
		)
		{
			OutPosition = mul(WorldPosition, WorldToClipMatrix);

			float3 LightDir = float3(WorldToShadowMatrix[0].z,WorldToShadowMatrix[1].z,WorldToShadowMatrix[2].z);
			float Nol = abs(dot(LightDir,WorldVertexNormal));

			const float MaxSlopeDepthBias = ShadowParams.z;
			//tan(theta) = sqrt(1-cos^2)/cos
			const float Slope = clamp(Nol>0?sqrt(saturate(1-Nol*Nol)) /Nol:MaxSlopeDepthBias,0,MaxSlopeDepthBias);

			//
			const float SlopeDepthBias = ShadowParams.y;
			const float SlopeBias = SlopeDepthBias * Slope;

			const float ConstantDepthBias = ShadowParams.x;
			const float DepthBias = SlopeBias + ConstantDepthBias;

			const float InvMaxSubjectDepth = ShadowParams.w;
			ShadowDepth = OutPosition.z * InvMaxSubjectDepth + DepthBias;
			OutPosition.z = ShadowDepth * OutPosition.w;
		}


		void Main(
		in float3 Position:POSITION,
		in float4 Tangent_Quat:TANGENT,
		out float4 OutPosition:SV_POSITION)
		{
			float4	WorldPos = float4(Position,1);

			Tangent_Quat = Tangent_Quat * 2 - 1;

			float3 WorldNormal =  transform_quat(float3(0, 0, 1), Tangent_Quat);

			float ShadowDepth;

			CalcShadowDepthOutput(
				ProjectionMatrix,
				ViewMatrix,
				WorldPos,
				WorldNormal,
				OutPosition,
				ShadowDepth
			);
		}
	")
)