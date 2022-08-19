#version 460
//#extension GL_ARB_shader_draw_parameters : enable

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;

layout(std140, binding = 0)uniform transformMatrix{
	mat4 viewMat;
	mat4 projMat;
	mat4 vpMat;
};

layout(std140, binding = 1)uniform modelDataList{
	mat4 modelMat;
	uint isDraw;
	uint modelID;
	uint materialID;
	uint tempData1;
};

struct MeshletData{
	vec4 sphere;
	vec3 color;
	uint clusterID;
	vec3 coneNorm;
	uint isDraw;
	vec3 coneApex;
	float coneAngle;
};

layout(std430, binding = 0) readonly buffer MeshletsDataBuffer{
	MeshletData meshletDatas[];
};

out VS_OUT{
	vec3 FragPos;
	vec3 Normal;
	vec2 uv;
	vec3 Color;
}vs_out;

void main(){
	vec4 worldPos = modelMat * vec4(position,1.0f);
	gl_Position = vpMat * worldPos;

	vs_out.FragPos = worldPos.xyz;
	vs_out.Normal = normal;
	vs_out.uv = texCoord;
	vs_out.Color = meshletDatas[gl_BaseInstance].color;
}