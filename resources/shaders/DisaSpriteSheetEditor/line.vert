#version 330

layout (location = 0) in vec4 vertex;

uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform mat4 modelMatrix;

void main()
{
	mat4 view = viewMatrix;
	view[0][0] = 1;
	view[0][1] = 0;
	view[0][2] = 0;
	
	view[1][0] = 0;
	view[1][1] = 1;
	view[1][2] = 0;
	
	view[2][0] = 0;
	view[2][1] = 0;
	view[2][2] = 1;
	
  gl_Position = projectionMatrix  * view * modelMatrix * vertex;
}