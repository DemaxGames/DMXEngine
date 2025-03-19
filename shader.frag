#version 450

layout(location = 0) in vec3 normal;

layout(location = 0) out vec4 outColor;

void main(){
	outColor = vec4(vec3(1.0, 1.0, 0.0), 1.0) * ((dot(normal * 0.3, -(vec3(0.0, 0.0, 1.0)) * 0.5) + 0.5));
}
