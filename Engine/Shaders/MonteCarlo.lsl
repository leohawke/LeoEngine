(effect
(shader
"
float2 UniformSampleDiskConcentric( float2 E )
{
	float2 p = 2 * E - 1;
	float Radius;
	float Phi;
	if( abs( p.x ) > abs( p.y ) )
	{
		Radius = p.x;
		Phi = (PI/4) * (p.y / p.x);
	}
	else
	{
		Radius = p.y;
		Phi = (PI/2) - (PI/4) * (p.x / p.y);
	}
	return float2( Radius * cos( Phi ), Radius * sin( Phi ) );
}
"
)
)
