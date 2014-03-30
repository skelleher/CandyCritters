precision highp float;

attribute vec4          aPosition;
attribute vec4          aDiffuse;
attribute vec2          aTextureCoordinate0;

uniform   mat4          uMatModelView;
uniform   mat4          uMatProjection;

varying   vec2          vTextureCoordinate0;
varying   vec4          vDiffuse;

varying   mediump mat2  vRotate;
varying   mediump mat2  vInverseRotate;
varying   mediump vec2  vHalfLineDistance;
varying   mediump float vDelta;
varying   mediump vec2  vHalftoneResolution;


/* Divide in this many halftone dots along x and y.
 * TODO: draw ellipse when x and y is different */
//uniform ivec2 halftoneResolution;
const vec2 halftoneResolution = vec2( 125.0, 125.0 );

/* Angle of halftone grid (degrees; positive = counterclockwise) */
const float angle = 45.0;

/* smoothness black to white (pseudo anti-aliasing). */
const float smooth = 0.2;


void main()
{
    // Transform the vertex into screen space
    gl_Position = uMatProjection * uMatModelView * vec4(aPosition.xyz, 1.0);

    // Pass to fragment shader
    vTextureCoordinate0  = aTextureCoordinate0;
    vDiffuse             = aDiffuse;
   
   
	vRotate         = mat2(cos(angle), -sin(angle),
	                       sin(angle), cos(angle));

	vInverseRotate  = mat2(cos(angle), sin(angle),
	                      -sin(angle), cos(angle));


	/* Distance to next dot divided by two. */ 
	vHalfLineDistance   = vec2(1.0)/vec2(halftoneResolution)/vec2(2.0);
	vDelta              = smooth * pow(vHalfLineDistance.x, 2.0);
    vHalftoneResolution = halftoneResolution;
}

