#version 330

layout (location = 0) in vec4 vertex;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uviChannel0;
layout (location = 3) in vec2 uviChannel1;

uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform mat4 modelMatrix;

out vec2 passUViChannel0;
out vec2 passUViChannel1;

void main()
{
  gl_Position = projectionMatrix * viewMatrix * modelMatrix * vertex;
  passUViChannel0 = uviChannel0;
  passUViChannel1 = uviChannel1;
}