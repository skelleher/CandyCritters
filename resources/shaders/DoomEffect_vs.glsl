precision highp float;

attribute vec4      aPosition;
attribute vec4      aDiffuse;
attribute vec2      aTextureCoordinate0;

uniform   mat4      uMatModelView;
uniform   mat4      uMatProjection;
uniform   vec3      uOrigin;
uniform   float     uTexelsPerColumn;

const     int       MAX_COLUMNS = 240;
uniform   float     uOffsets[MAX_COLUMNS];

varying   vec2      vTextureCoordinate0;
varying   vec4      vDiffuse;


void main()
{
    // Which bucket does this vertex fall into?
    int   column   = int(aPosition.x / uTexelsPerColumn);
    float progress = uOffsets[column];
    
    vec4 vertex = aPosition;
    vertex.y = mix( aPosition.y, uOrigin.y, progress );

    // Transform the vertex into screen space
    gl_Position = uMatProjection * uMatModelView * vec4(vertex.xyz, 1.0);


    // Pass to fragment shader
    vDiffuse              = aDiffuse;
    vTextureCoordinate0   = aTextureCoordinate0;
}

