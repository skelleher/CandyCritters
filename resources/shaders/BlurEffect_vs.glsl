attribute highp vec4     aPosition;
attribute highp vec4     aDiffuse;
attribute highp vec2     aTextureCoordinate0;

uniform   highp mat4     uMatModelView;
uniform   highp mat4     uMatProjection;

varying   mediump  vec2     vTextureCoordinate0;
varying   mediump  vec4     vDiffuse;


void main()
{
   // Transform the vertex into screen space
   gl_Position = uMatProjection * uMatModelView * vec4(aPosition.xyz, 1.0);

   // Pass to fragment shader
   vDiffuse              = aDiffuse;
   vTextureCoordinate0   = aTextureCoordinate0;
}

