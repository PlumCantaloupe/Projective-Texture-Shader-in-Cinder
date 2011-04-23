#version 120

//uniform sampler2D baseTex;
//
//varying vec2 uv;
//
//void main()
//{
//	vec4 baseTex = texture2D( baseTex, uv );
//
//	gl_FragColor = baseTex;
//}

const float C_PI    = 3.1415;
const float C_2PI   = 2.0 * C_PI;
const float C_2PI_I = 1.0 / (2.0 * C_PI);
const float C_PI_2  = C_PI / 2.0;

uniform float StartRad;
uniform vec2  Freq;
uniform vec2  Amplitude;

uniform sampler2D baseTex;

varying vec2 uv;

void main (void)
{
	vec2  perturb;
	float rad;
	vec4  color;

	// Compute a perturbation factor for the x-direction
	rad = (uv.s + uv.t - 1.0 + StartRad) * Freq.x;

	// Wrap to -2.0*PI, 2*PI
	rad = rad * C_2PI_I;
	rad = fract(rad);
	rad = rad * C_2PI;

	// Center in -PI, PI
	if (rad >  C_PI) rad = rad - C_2PI;
	if (rad < -C_PI) rad = rad + C_2PI;

	// Center in -PI/2, PI/2
	if (rad >  C_PI_2) rad =  C_PI - rad;
	if (rad < -C_PI_2) rad = -C_PI - rad;

	perturb.x  = (rad - (rad * rad * rad / 6.0)) * Amplitude.x;

	// Now compute a perturbation factor for the y-direction
	rad = (uv.s - uv.t + StartRad) * Freq.y;

	// Wrap to -2*PI, 2*PI
	rad = rad * C_2PI_I;
	rad = fract(rad);
	rad = rad * C_2PI;

	// Center in -PI, PI
	if (rad >  C_PI) rad = rad - C_2PI;
	if (rad < -C_PI) rad = rad + C_2PI;

	// Center in -PI/2, PI/2
	if (rad >  C_PI_2) rad =  C_PI - rad;
	if (rad < -C_PI_2) rad = -C_PI - rad;

	perturb.y  = (rad - (rad * rad * rad / 6.0)) * Amplitude.y;

	color = texture2D(baseTex, perturb + uv.st);
	gl_FragColor = color;
} 