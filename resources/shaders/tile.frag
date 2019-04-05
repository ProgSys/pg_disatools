#version 330 core

in vec2 passUViChannel0;
in vec2 passUViChannel1;

uniform sampler2D iChannel0;
uniform sampler2D iChannel1;
uniform sampler2D shadowmap;
uniform int blendMode;

out vec4 fragmentColor;
void main() {
	vec4 t0 = texture(iChannel0,passUViChannel0);
	vec4 t1 = texture(iChannel1,passUViChannel1);
	
	//multiply
	t1.a *= 0.5;
	if(t1.a == 0)
		fragmentColor = t0;
	else{
		float a = 1-t1.a;
		fragmentColor = vec4(
			t0.r*min(t1.r+a,1),
			t0.g*min(t1.g+a,1),
			t0.b*min(t1.b+a,1),
			1
		);
	}
	

	//fragmentColor = texture(iChannel1,passUViChannel1);
	//fragmentColor.a = 1.0;
}