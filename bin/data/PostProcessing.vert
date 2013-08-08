attribute vec4 position;

uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;


void main()
{
	vec4 pos = projectionMatrix * modelViewMatrix * position;
	gl_Position = pos;

}