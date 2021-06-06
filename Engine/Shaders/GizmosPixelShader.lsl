(effect
	(shader
	"
		void Main(
		in float4 ClipPos : SV_POSITION,
		in float4 InColor : COLOR,
		out float4 OutColor : SV_TARGET
		)
		{
			OutColor = InColor;
		}
	"
	)
)