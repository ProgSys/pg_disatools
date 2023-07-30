#version 330 core

in vec2 passTcoord;

uniform sampler2D idtexture;
uniform sampler2D colorTable;

uniform int colortableStart;
uniform float alphaMult;
uniform int selectedID;

out vec4 fragmentColor;
void main() {
	float spriteID = texture(idtexture, passTcoord).r;
	int id = int(spriteID*255);
	if(selectedID == id){
		fragmentColor = vec4(1,0,0,1);
	}else{
		fragmentColor = texelFetch(colorTable,ivec2(colortableStart + id, 0 ),0);
		fragmentColor.a *= alphaMult;
		if(fragmentColor.a == 0) discard;
	}
}