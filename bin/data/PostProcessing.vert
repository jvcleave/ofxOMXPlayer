attribute vec4 position;
attribute vec4 color;
attribute vec4 normal;
attribute vec2 texcoord;

uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;

varying vec4 colorVarying;
varying vec2 texCoordVarying;


void main()
{
	vec4 pos = projectionMatrix * modelViewMatrix * position;
	gl_Position = pos;
	
	colorVarying = color;
	texCoordVarying = texcoord;
}