attribute highp   vec4  aPosition;
attribute lowp    vec4  aDiffuse;
attribute lowp    vec2  aTextureCoordinate0;

uniform   highp   mat4  uMatModelView;
uniform   highp   mat4  uMatProjection;
uniform   highp   vec2  uOrigin;
uniform   mediump float uAmplitude;
uniform   mediump float uHalfWaveLength;
uniform   mediump float uRadius;
uniform   mediump float uNumWaves;
uniform   lowp    vec4  uColor;
uniform   mediump float uTime;

varying   lowp vec2     vTextureCoordinate0;
varying   lowp vec4     vDiffuse;
varying   lowp vec4     vWaveColor;


const highp float PI     = 3.1415926;
const highp float TWO_PI = PI*2.0;


highp float rand(vec2 v)
{
    return fract(sin(dot(v.xy ,vec2(12.9898,78.233))) * 43758.5453);
}


void main()
{
    highp vec4  vertex  = aPosition;
    highp float dist    = distance(aPosition.xy, uOrigin.xy);
    
/*
    if (uRadius > 0.0)
    {
        //
        // Don't generate waves inside or outside of the current radius;
        // creates a travelling wave effect when radius is animated.
        //
        if (dist < uRadius - uHalfWaveLength)
        {
            dist = 0.0;
        }
        else if (dist > uRadius + uHalfWaveLength)
        {
            dist = 0.0;
        }

//        dist = smoothstep(uRadius - uHalfWaveLength, uRadius + uHalfWaveLength, dist);
    }
*/

    // This version works for standing waves:
//    float deflection = uAmplitude * sin( (dist*uFrequency) /*- uTime*/ );

    // This version works for a single traveling wave:
//    float deflection = uAmplitude * sin( dist * TWO_PI * uNumWaves );

    
    // The distance contribution.
    mediump float deflection = sin( (dist / (2.0*uHalfWaveLength)) * 1.0  - uTime );

    // The travelling wave contribution.
    if (uRadius > 0.0)
    {
        if (dist < uRadius)
        {
            deflection *= (dist/uRadius);
        }
        else
        {
            deflection *= (uRadius/dist);
        }
        
        // Travelling waves look best if they don't go negative.
        deflection = abs(deflection);
    }

    deflection *= uAmplitude;
 
 /*
    // TEST:
    {
    vec2 seed = vec2(aPosition.x, aPosition.y);
    deflection = deflection * rand(seed);
    }
 */
    
    vertex.z += deflection;

    // Transform the vertex into screen space
    gl_Position = uMatProjection * uMatModelView * vec4(vertex.xyz, 1.0);

    // Pass to fragment shader
    vDiffuse             = aDiffuse;
    vTextureCoordinate0  = aTextureCoordinate0;

//    vWaveColor = uColor * (deflection / uAmplitude);
}
