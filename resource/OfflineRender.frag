#version 450
in VS_OUT{
	vec3 FragPos;
	vec3 Normal;
	vec2 uv;
	vec3 Color;
}fs_in;

layout(location = 0)out vec4 OfflineCA0;
layout(location = 1)out float OfflineCA1;

layout(std140, binding = 0)uniform transformMatrix{
	mat4 viewMat;
	mat4 projMat;
	mat4 vpMat;
};
layout(std140, binding = 2) uniform MaterialData{
	float diffuse;
	float specular;
	float ambient;
	float amplify;
};
layout(location = 0)uniform vec3 cameraPos;
void main(){
	vec3 LightDirection = vec3(2.0f, -2.0f, 1.0f);
	vec3 LightDiffuse = vec3(diffuse);
	vec3 LightSpecular = vec3(specular);
	vec3 LightAmbient = vec3(ambient);

	vec3 normal = normalize(fs_in.Normal);
	vec3 LightDir = normalize(LightDirection);
	float Diff = max(dot(normal, LightDir), 0.0f);
	vec3 Diffuse = LightDiffuse * Diff;

	vec3 viewDir = normalize(cameraPos - fs_in.FragPos);
	vec3 reflectDir = reflect(-LightDir, normal);
	float Spec = pow(max(dot(viewDir, reflectDir), 0.0f), 32.0f);
	vec3 Specular = LightSpecular * Spec;

	vec3 Ambient = LightAmbient;

	vec3 result = (Diffuse + Specular + Ambient) * fs_in.Color;

	OfflineCA0 = vec4(result, 1.0f);
	OfflineCA1 = float(gl_FragCoord.z);
}