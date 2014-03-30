// lowp is dramatically faster than medium or highp on the iPhone
precision lowp float;

uniform   sampler2D     uTexture;
uniform   bool          uTextureEnabled;
uniform   bool          uLightingEnabled;
uniform   bool          uPointSpriteEnabled;
uniform   vec4          uGlobalColor;

varying   vec4          vDiffuse;
varying   vec2          vTextureCoordinate0;


void main(void)
{
    if (uTextureEnabled)
    {
        if (uPointSpriteEnabled)
        {
            gl_FragColor   = texture2D(uTexture, gl_PointCoord) * vDiffuse;
        }
        else
        {
            gl_FragColor   = texture2D(uTexture, vTextureCoordinate0) * vDiffuse;
        }
    }
    else
    {
        gl_FragColor = vDiffuse;
    }

    // Draw black outline around point-sprites for debugging.
    if (uPointSpriteEnabled)
    {
        if(gl_PointCoord.x < 0.1 || gl_PointCoord.x > 0.9 || gl_PointCoord.y < 0.1 || gl_PointCoord.y > 0.9)
        {
            gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
        }
    }
}

