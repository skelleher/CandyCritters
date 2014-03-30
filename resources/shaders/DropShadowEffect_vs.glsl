attribute highp   vec4  aPosition;
attribute lowp    vec4  aDiffuse;
attribute lowp    vec2  aTextureCoordinate0;

uniform   highp   mat4  uMatModelView;
uniform   highp   mat4  uMatProjection;

uniform   lowp    vec4  uColor;
uniform   mediump float uDepth;
uniform   mediump float uDirectionDegrees;

varying   lowp    vec2  vTextureCoordinate0;
varying   lowp    vec4  vDiffuse;


void main()
{
    // Transform the vertex into modelView space.
    gl_Position = uMatModelView * vec4(aPosition.xyz, 1.0);


    // Apply the shadow offset
    mediump vec4 offset = vec4(0.0, 0.0, 0.0, 0.0);
    offset.x       = (uDepth * cos(uDirectionDegrees / 180.0 * 3.14159));
    offset.y       = (uDepth * sin(uDirectionDegrees / 180.0 * 3.14159));

    gl_Position += offset;


    // Transform the vertex into screen space.
    gl_Position = uMatProjection * gl_Position;
    
    
    // Pass to fragment shader
    vDiffuse              = uColor;
    vDiffuse.a            = min(uColor.a, aDiffuse.a);
    vTextureCoordinate0   = aTextureCoordinate0;
}

