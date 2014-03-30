precision highp float;

attribute vec4      aPosition;
attribute vec4      aDiffuse;
attribute vec2      aTextureCoordinate0;

uniform   mat4      uMatModelView;
uniform   mat4      uMatProjection;

varying   vec2      vTextureCoordinate0;
varying   vec4      vDiffuse;


void main()
{
   // Transform the vertex into screen space
   gl_Position = uMatProjection * uMatModelView * vec4(aPosition.xyz, 1.0);

   // Pass to fragment shader
   vDiffuse              = aDiffuse;
   vTextureCoordinate0   = aTextureCoordinate0;
}

