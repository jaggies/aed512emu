#version 120
attribute vec4 position;
attribute vec4 uv;
void main() {
	gl_Position = gl_ProjectionMatrix * gl_ModelViewMatrix * position;
	gl_TexCoord[0] = gl_TextureMatrix[0] * uv; //gl_MultiTexCoord0
}
