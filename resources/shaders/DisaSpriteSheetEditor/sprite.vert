#version 330

layout (location = 0) in vec4 vertex;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uv;

uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform mat4 modelMatrix;

uniform vec2 spriteSize;
uniform vec2 start;
uniform vec2 size;
uniform vec2 mirror;

out vec2 passTcoord;

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
	
	
	gl_Position = projectionMatrix * view * modelMatrix * vertex;

	//passTcoord = vec2( (start.x/spriteSize.x)+( (size.x)/spriteSize.x) * abs(mirror.x - uv.x), (start.y/spriteSize.y)+((size.y)/spriteSize.y)* abs( mirror.y - uv.y));
	passTcoord = vec2((start.x/size.x) + ( spriteSize.x/size.x) * abs(mirror.x-uv.x), (start.y/size.y)+ ( spriteSize.y/size.y) * abs(mirror.y-uv.y));
}