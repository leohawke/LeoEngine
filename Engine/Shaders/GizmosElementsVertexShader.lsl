(effect
	(shader
	"
		float4x4 Transform;

		void Main(
		in float4 InPosition : POSITION,
		in float4 InColor : COLOR,
		out float4 ClipPos : SV_POSITION,
		out float4 OutColor :COLOR
		)
		{
			ClipPos = mul(InPosition,Transform);
			OutColor = InColor;
		}
	"
	)
)