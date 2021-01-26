#version 400

in vec2 texCoords;

out vec4 fragColor;

uniform sampler2D ourTexture;

void main() {
  fragColor = texture(ourTexture, texCoords);
}
