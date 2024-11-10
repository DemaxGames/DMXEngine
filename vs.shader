#version 330 core

in float position;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

void main() {
    int x = (int(position) >> 8) & 255;
    int y = int(position) & 255;
    x = x - 128;
    y = y - 128;
	gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(float(x)/2.0, float(y)/2.0, 0.0f, 1.0f);
}