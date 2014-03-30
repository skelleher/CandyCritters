// lowp is dramatically faster than medium or highp on the iPhone
//precision lowp float;

uniform   sampler2D     uTexture;
uniform   bool          uTextureEnabled;
uniform   bool          uLightingEnabled;
uniform   bool          uPointSpriteEnabled;
uniform   lowp vec4     uGlobalColor;

varying   lowp vec4     vDiffuse;
varying   lowp vec2     vTextureCoordinate0;

void main(void)
{
    gl_FragColor = uGlobalColor * texture2D(uTexture, vTextureCoordinate0) * vDiffuse;
}



