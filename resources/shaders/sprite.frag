#version 330 core

in vec2 passTcoord;

uniform sampler2D idtexture;
uniform sampler2D colorTable;

uniform int colortableStart;
uniform float alphaMult;

out vec4 fragmentColor;
void main() {
	float spriteID = texture(idtexture, passTcoord).r;
	
	fragmentColor = texelFetch(colorTable,ivec2(colortableStart + spriteID*255, 0 ),0);
	fragmentColor.a *= alphaMult;
	if(fragmentColor.a == 0) discard;
}