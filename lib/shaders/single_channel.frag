uniform sampler1D lut;
uniform sampler2D image;
void main() {
	vec4 raw = texture2D(image, vec2(gl_TexCoord[0][0], gl_TexCoord[0][1]));
	gl_FragColor = texture1D(lut, raw[0]);
}
