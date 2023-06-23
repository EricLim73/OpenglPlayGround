#version 460 core

in vec4 wPos;
in vec4 wNorm; 
in vec3 TexCoord; // v3
out vec4 FragColor;

struct Light {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
uniform Light light;

struct Material {
	samplerCube diffuse;
    samplerCube specular;
    samplerCube emission;

    float shininess;
};
uniform Material material;

uniform vec3 viewPos;

void main() {
	// NOTE: just to make it so that the opposite side don't so dark.
	vec3 texColor = min(1.5 * texture(material.diffuse, TexCoord).rgb, 
						vec3(1.0f, 1.0f, 1.0f));
  	vec3 ambient = texColor * light.ambient;
 
    vec3 lightDir = normalize(light.position - wPos.xyz);
    vec3 pixelNorm = normalize(wNorm.xyz);
    float diff = max(dot(pixelNorm, lightDir), 0.0);
	vec3 diffuse = diff * texColor * light.diffuse;
	// TODO: (Ericlim73) For now setting it to 1.
	vec3 specColor = vec3(1.0f, 1.0f, 1.0f);
    vec3 viewDir = normalize(viewPos - wPos.xyz);
    vec3 reflectDir = reflect(-lightDir, pixelNorm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = spec * specColor * light.specular;
	
    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
}