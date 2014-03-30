// lowp is dramatically faster than medium or highp on the iPhone
precision highp float;

uniform   sampler2D     uTexture;
uniform   bool          uTextureEnabled;
uniform   bool          uLightingEnabled;
uniform   vec4          uColor;
uniform   vec4          uGlobalColor;

varying   vec4          vDiffuse;
varying   vec2          vTextureCoordinate0;


const float C_PI    = 3.1415926;
const float C_2PI   = 2.0 * C_PI;
const float C_2PI_I = 1.0 / (C_2PI);
const float C_PI_2  = C_PI / 2.0;

const float uStartRad   = 0.0;
const vec2  uFreq       = vec2(2.0, 2.0);
const vec2  uAmplitude  = vec2(0.05, 0.05);


void main(void)
{
    gl_FragColor = texture2D( uTexture, vTextureCoordinate0 ) * vDiffuse;

/*
    vec2 perturb;
    float rad;
    
    rad = (vTextureCoordinate0.x + vTextureCoordinate0.y - 1.0 + uStartRad) + uFreq.x;

    rad = rad * C_2PI_I;
    rad = fract(rad);
    rad = rad * C_2PI;
    
    if (rad >  C_PI) rad = rad - C_2PI;
    if (rad < -C_PI) rad = rad + C_2PI;
    
    if (rad >  C_PI_2) rad =  C_PI - rad;
    if (rad < -C_PI_2) rad = -C_PI - rad;
    
//    perturb.x = (rad - (rad * rad * rad / 6.0)) * uAmplitude.x;
    perturb.x = sin(rad) * uAmplitude.x;
    
    rad = (vTextureCoordinate0.x - vTextureCoordinate0.y + uStartRad) * uFreq.y;
    
    rad = rad * C_2PI_I;
    rad = fract(rad);
    rad = rad * C_2PI;
    
    if (rad >  C_PI) rad = rad - C_2PI;
    if (rad < -C_PI) rad = rad + C_2PI;
    
    if (rad > C_PI_2) rad =  C_PI - rad;
    if (rad < C_PI_2) rad = -C_PI - rad;
    
//    perturb.y = (rad - (rad * rad * rad / 6.0)) * uAmplitude.y;
    perturb.y = sin(rad) * uAmplitude.y;
    
    gl_FragColor = vec4(texture2D( uTexture, perturb + vTextureCoordinate0).xyz, 1.0);
*/
}
