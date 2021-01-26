#version 400

in vec2 texCoords;

out vec4 fragColor;

uniform sampler2D ourTexture;

void main() {
  vec4 texel = texture(ourTexture, texCoords);
  fragColor.r = texel.r * 0.393 + texel.g * 0.769 + texel.b * 0.189;
  fragColor.g = texel.r * 0.349 + texel.g * 0.686 + texel.b * 0.168;
  fragColor.b = texel.r * 0.272 + texel.g * 0.534 + texel.b * 0.131;
  fragColor.a = texel.a;
}
