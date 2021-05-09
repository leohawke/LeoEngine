(effect
	(shader
	"
		float4x4 ProjectionMatrix;


		void Main(in float3 Postion:POSITION,
		out float4 ClipPos:SV_POSITION)
		{
			ClipPos = mul(float4(Position,1),ProjectionMatrix);
		}
	")
)