#version 450

layout(location = 0) in vec2 uv;

layout(location = 0) out vec4 FragColor;

layout(binding = 0) uniform sampler2D OfflineCA0;

void main(){
	vec3 color = texture(OfflineCA0, uv).xyz;
	FragColor = vec4(color, 1.0f);
}