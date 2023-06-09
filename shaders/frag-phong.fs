#version 460 core

in vec4 vPos;
in vec4 vNorm; 

out vec4 FragColor;

struct Light {
	vec4 pos;
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	vec4 att;
};
uniform Light light;

struct Material {
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	float shineness;
};
uniform Material material;

uniform vec4 viewPos;


void main(void) { 
	vec3 N = normalize(vNorm.xyz);
    vec3 L = normalize(light.pos.xyz - vPos.xyz); 
    vec3 V = normalize(vPos.xyz - viewPos.xyz); 
    vec3 R = reflect(-L, N);

	float d = length(light.pos.xyz - vPos.xyz);
	float denom = light.att.x + light.att.y * d + light.att.z * d * d;

	vec4 ambient = light.ambient * material.ambient;
	vec4 diffuse = light.diffuse * max(dot(L, N), 0.0) *  (material.diffuse / denom);
	vec4 specular = light.specular * pow(max(dot(R, V), 0.0), material.shineness) * (material.specular / denom);
	
    FragColor = ambient + diffuse + specular;
}