#version 450

layout(location = 0) in vec3 position;



layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 transform;
    mat4 perspective;
} ubo;

layout(location = 0) out vec3 normal;

void main(){
	gl_Position = ubo.perspective * ubo.transform * ubo.model * vec4(position, 1.0);
    normal = (ubo.perspective * ubo.transform * ubo.model * vec4(position, 1.0)).xyz;
}
