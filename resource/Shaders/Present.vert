#version 450

layout(location = 0) out vec2 uv;

void main(){
	vec4 pos = vec4(
		(float(gl_VertexID & 1)) * 4.0f - 1.0f,
		(float((gl_VertexID >> 1) & 1)) * 4.0f - 1.0f,
		0.0f, 1.0f
		);
	uv = pos.xy * 0.5f + 0.5f;
	gl_Position = pos;
}