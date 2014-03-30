attribute highp vec4    aPosition;
attribute lowp  vec4    aDiffuse;
attribute lowp  vec2    aTextureCoordinate0;

uniform   highp mat4    uMatModelView;
uniform   highp mat4    uMatProjection;
uniform   highp float   uPointSpriteSize;
uniform   bool          uPointSpriteEnabled;

varying   lowp  vec2    vTextureCoordinate0;
varying   lowp  vec4    vDiffuse;


void main()
{
    // Transform the vertex into screen space
    gl_Position = uMatProjection * uMatModelView * vec4(aPosition.xyz, 1.0);

    if (uPointSpriteEnabled)
    {
        gl_PointSize = uPointSpriteSize;
    }

    // Pass to fragment shader
    vDiffuse              = aDiffuse;
    vTextureCoordinate0   = aTextureCoordinate0;
}

