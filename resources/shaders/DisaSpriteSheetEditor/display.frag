#version 330 core

in vec2 passTcoord;

uniform sampler2D imagetexture;

out vec4 fragmentColor;
void main() {
	fragmentColor = texture(imagetexture, passTcoord);
}