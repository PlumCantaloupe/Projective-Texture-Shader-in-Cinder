#version 120

varying vec2 uv;

void main()
{
	gl_Position = ftransform();
	gl_Position = sign( gl_Position );
    
	uv = (vec2( gl_Position.x, gl_Position.y ) + vec2( 1.0 ) ) * 0.5 ;

	// Texture coordinate for screen aligned (in correct range):
//	//gl_Position = ftransform();
//	gl_Position = sign( gl_Position.xy );
//	
//	uv = vec2(gl_Position.x, gl_Position.y);
//
//	gl_TexCoord[0] = gl_MultiTexCoord0;
//	gl_TexCoord[1] = gl_MultiTexCoord1;
//	gl_Position = ftransform();
}
