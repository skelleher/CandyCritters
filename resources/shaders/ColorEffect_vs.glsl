attribute highp vec4      aPosition;
attribute lowp  vec4      aDiffuse;
attribute lowp  vec2      aTextureCoordinate0;

uniform   highp mat4      uMatModelView;
uniform   highp mat4      uMatProjection;
uniform   highp vec2      uOrigin;
uniform   highp float     uRadius;
uniform   lowp  float     uBlend;

varying   lowp vec2      vTextureCoordinate0;
varying   lowp vec4      vDiffuse;
varying   lowp float     vBlend;


void main()
{
    // Transform the vertex into screen space
    gl_Position = uMatProjection * uMatModelView * vec4(aPosition.xyz, 1.0);

    highp vec4 vertex     = aPosition;
    highp float dist      = distance(aPosition.xy, uOrigin.xy);


    // Pass to fragment shader
    vDiffuse              = aDiffuse;
    vTextureCoordinate0   = aTextureCoordinate0;
    vBlend                = dist < uRadius ? uBlend : 0.0;
}

