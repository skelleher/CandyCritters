precision mediump float;

uniform   sampler2D         uTexture;
uniform   bool              uTextureEnabled;
uniform   bool              uLightingEnabled;
uniform   vec4              uGlobalColor;

varying   vec2              vTextureCoordinate0;
varying   vec4              vDiffuse;

varying   mediump mat2      vRotate;
varying   mediump mat2      vInverseRotate;
varying   mediump vec2      vHalfLineDistance;
varying   mediump float     vDelta;
varying   mediump vec2      vHalftoneResolution;


/* Contrast control */
const float contrastDelta   = 0.3;          // higher -> grey gets darker
const float brightness      = 0.0;          // analog for white
const float blackness       = 1.1;          // higher -> larger areas completely covered by dots
const float SQRT_2          = 1.41421356;

const vec4 dotColor        = vec4(0.0, 0.0, 0.0, 1.0);
const vec4 backgroundColor = vec4(1.0, 1.0, 1.0, 0.0);


void main(void)
{
	/* Find center of the halftone dot. */
	vec2 center = vRotate * vTextureCoordinate0;
//    vec2 center = vTextureCoordinate0;
	center      = floor(center * vHalftoneResolution) / vHalftoneResolution;
	center      += vHalfLineDistance;
	center      = vInverseRotate * center;

	/* Only green (texture is grey scale) */
//	float luminance = texture2D(uTexture, center).g;
    float luminance = dot(vec3(0.222, 0.707, 0.071), texture2D(uTexture, center).rgb);

	/* Radius of the halftone dot. */
	float radius = SQRT_2 * vHalfLineDistance.x * (1.0 - luminance) * blackness;

//	float contrast = 1.0 + (contrastDelta)/(2.0);
//	float radiusSqrd = contrast * (radius*radius)
//		- (contrastDelta * vHalfLineDistance.x * vHalfLineDistance.x)/2.0
//		- brightness * vHalfLineDistance.x * vHalfLineDistance.x;

    float radiusSqrd = radius*radius;

    vec2  distance   = abs(center - vTextureCoordinate0);
    vec2  power      = distance*distance;
	float pixelDist2 = power.x + power.y; // Distance pixel to center squared.

//	float gradient = smoothstep(radiusSqrd - vDelta, radiusSqrd + vDelta, pixelDist2);
    float gradient = step(radiusSqrd, pixelDist2);
	gl_FragColor = mix(dotColor, backgroundColor, gradient);
    gl_FragColor.a = vDiffuse.a;
}
