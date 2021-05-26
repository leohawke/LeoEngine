(effect
	(shader
	"
		void Main(
		in float4 InPosition : POSITION,
		out float4 OutPosition : SV_POSITION
		)
		{
			// Pass position straight through, as geometry is defined in clip space already.
			OutPosition = float4(InPosition.xyz, 1);
		}
	"
	)
)