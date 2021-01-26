#version 400

in vec2 position;

out vec2 texCoords;

uniform mat4 projectionMatrix;
uniform mat4 modelMatrix;

void main() {
  gl_Position = projectionMatrix * modelMatrix * vec4(position, 0.0, 1.0);
  texCoords = position;
}
