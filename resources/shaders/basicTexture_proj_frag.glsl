//warning: blending code is pretty terrible; but the idea is there ...

#version 120

uniform sampler2D diffuseTex;
uniform sampler2D projMap;
uniform float alpha;

varying vec4 diffuse,ambient;
varying vec3 normal,lightDir,halfVector;
varying vec3 viewDir;

void main()
{
	vec3 n,halfV,viewV,ldir;
	float NdotL,NdotHV;
	vec4 color = ambient;
	vec4 newDiffuse = texture2D( diffuseTex, gl_TexCoord[0].st );
	
	/* a fragment shader can't write a verying variable, hence we need
	a new variable to store the normalized interpolated normal */
	n = normalize(normal);
	
	/* compute the dot product between normal and ldir */
	NdotL = max(dot(n,lightDir),0.5); //hacking b/c normals are fucked up on fish: should be greater than 0.0 ... 0.5 is hacked num

	vec3 reflectVec = -normalize( reflect( lightDir, normal ) );
	vec4 specular = gl_FrontLightProduct[0].specular * pow(max(dot(reflectVec, viewDir), 0.0), gl_FrontMaterial.shininess);

	if (NdotL > 0.0) 
	{
		halfV = normalize(halfVector);
		NdotHV = max(dot(n,halfV),0.0);
		//out atm as there are noi materials assigned color += specular * pow(NdotHV,gl_FrontMaterial.shininess);
		color += newDiffuse * NdotL;
	}
	
	//project but suppress reverse projection
    if( gl_TexCoord[0].q > 0.0 )
    {
		vec4 ProjMapColor = texture2D(projMap, fract(gl_TexCoord[1].st / vec2(5, 5)) );  //tiling and scaling:vec2(10, 10)
		ProjMapColor.a = ProjMapColor.a * 0.2; //tone down alpha
		ProjMapColor = ProjMapColor * ProjMapColor.a; //account for alpha
		if ( ProjMapColor.r < 0 ) ProjMapColor *= 0;
		color += ProjMapColor;			
    }
	
	vec4 alphaVec = vec4(0.0, 0.0, 0.0, ( 1.0 - alpha) );
	gl_FragColor = color - alphaVec;
}
