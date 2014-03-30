// lowp is dramatically faster than medium or highp on the iPhone
precision lowp float;

uniform   sampler2D     uTexture;
uniform   bool          uTextureEnabled;
uniform   bool          uLightingEnabled;
uniform   vec4          uGlobalColor;

varying   vec4          vDiffuse;
varying   vec2          vTextureCoordinate0;


void main(void)
{
//    if (uTextureEnabled)
//    {
        gl_FragColor   = uGlobalColor * texture2D(uTexture, gl_PointCoord) * vDiffuse;
//    }
//    else
//    {
//        gl_FragColor = vDiffuse;
//    }
}

