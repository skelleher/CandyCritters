//precision mediump float;

attribute highp vec4      aPosition;
attribute lowp  vec4      aDiffuse;
attribute lowp  vec2      aTextureCoordinate0;

uniform   highp mat4      uMatModelView;
uniform   highp mat4      uMatProjection;
uniform   highp vec3      uOrigin;

varying   lowp  vec2      vTextureCoordinate0;
varying   lowp  vec4      vDiffuse;

// Bezier curve 0
uniform   highp vec3      c0p0;
uniform   highp vec3      c0p1;
uniform   highp vec3      c0p2;
uniform   highp vec3      c0p3;

// Bezier curve 1
uniform   highp vec3      c1p0;
uniform   highp vec3      c1p1;
uniform   highp vec3      c1p2;
uniform   highp vec3      c1p3;

uniform   highp float     uSourceWidth;
uniform   highp float     uSourceHeight;


// For debug visualization:
varying   highp vec3      vLeft;
varying   highp vec3      vRight;
varying   highp vec3      vMiddle;


void main()
{
    // Changing this to mediump breaks the animation; why?
    lowp float T         = aPosition.y/uSourceHeight;
    lowp float oneMinusT = 1.0 - T;

    // Compute two cubic bezier curves.
    highp vec3 left  = pow(oneMinusT, 3.0)*c0p0 + 3.0*(pow(oneMinusT, 2.0))*T*c0p1 + 3.0*oneMinusT*pow(T, 2.0)*c0p2 + pow(T, 3.0)*c0p3;
    highp vec3 right = pow(oneMinusT, 3.0)*c1p0 + 3.0*(pow(oneMinusT, 2.0))*T*c1p1 + 3.0*oneMinusT*pow(T, 2.0)*c1p2 + pow(T, 3.0)*c1p3;


    // Interpolate the vertex position between both curves.
    highp vec3 vertex;
    vertex.x = mix(left.x, right.x, aPosition.x/uSourceWidth);
///    vertex.y = mix(left.y, right.y, aPosition.y/uSourceHeight);
    vertex.y = aPosition.y;
    vertex.z = mix(left.z, right.z, 0.5);
//    vertex.z = left.z;


    //highp vec3 vertex = aPosition.xyz;
    gl_Position = uMatProjection * uMatModelView * vec4(vertex.xyz, 1.0);

    // Pass to fragment shader
    vDiffuse             = aDiffuse;
    vTextureCoordinate0  = aTextureCoordinate0;
    vLeft                = left;
    vRight               = right;
    vMiddle              = mix(left, right, 0.5);
    
//    if (distance(aPosition.xyz, c0p0) <= 0.1 ||
//        distance(aPosition.xyz, c0p1) <= 0.1 ||
//        distance(aPosition.xyz, c0p2) <= 0.1 ||
//        distance(aPosition.xyz, c0p3) <= 0.1 ||
//        distance(aPosition.xyz, c1p0) <= 0.1 ||
//        distance(aPosition.xyz, c1p1) <= 0.1 ||
//        distance(aPosition.xyz, c1p2) <= 0.1 ||
//        distance(aPosition.xyz, c1p3) <= 0.1   )
//    {
//        vDiffuse = vec4(1, 0, 0, 1);
//    }
}
