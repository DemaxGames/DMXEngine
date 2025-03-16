#version 450

layout(location = 0) in vec2 position;

layout(binding = 0) uniform UniformBufferObject {
    mat4 rotation;
} ubo;
void main(){
	gl_Position = ubo.rotation * vec4(position, 0.0, 1.0);
}
