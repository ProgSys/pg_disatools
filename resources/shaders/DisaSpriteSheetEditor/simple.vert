#version 330

layout (location = 0) in vec4 vertex;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uv;

uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform mat4 modelMatrix;

out vec2 passTcoord;

void main()
{
  gl_Position = projectionMatrix * viewMatrix * modelMatrix * vertex;
  passTcoord = uv;
}