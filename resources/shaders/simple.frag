#version 330 core

in vec2 passTcoord;

uniform sampler2D texture01;

out vec4 fragmentColor;
void main() {
	fragmentColor = texture(texture01, passTcoord);
	//fragmentColor.a = 1;
}