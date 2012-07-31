// A simple vertex shader
#version 110

varying vec3 normal;
varying vec4 ecPosition;

varying vec4 texCoord0;
varying vec4 texCoord1;

void mrt()
{
	normal = gl_NormalMatrix * gl_Normal;

	ecPosition = gl_ModelViewMatrix * gl_Vertex;

	gl_FrontColor = gl_Color;
	gl_Position = ftransform();

	texCoord0 = gl_MultiTexCoord0;
   // shadow map coord
   texCoord1 = gl_TextureMatrix[1]*gl_Vertex;
}

void main()
{
	mrt();
}
