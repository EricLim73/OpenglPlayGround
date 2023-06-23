#version 460 core

in vec4 wPos;
in vec4 wNorm;
in vec2 wTexCoord;
out vec4 fragColor;

struct Light {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
uniform Light light;
 
struct Material {
	sampler2D diffuse;
    sampler2D specular;
    sampler2D emission;

    float shininess;
};
uniform Material material;

uniform vec3 viewPos;
uniform float timeStamp;

vec3 calculate_emission()
{
	vec3 show = step(vec3(1.0), vec3(1.0) - texture(material.specular, wTexCoord).rgb);
	return texture(material.emission, vec2(wTexCoord.x, wTexCoord.y + timeStamp)).rgb * show;
}

void main() {
	vec3 texColor = texture2D(material.diffuse, wTexCoord).rgb;
  	vec3 ambient = texColor * light.ambient;
 
    vec3 lightDir = normalize(light.position - wPos.xyz);
    vec3 pixelNorm = normalize(wNorm.xyz);
    float diff = max(dot(pixelNorm, lightDir), 0.0);
	vec3 diffuse = diff * texColor * light.diffuse;
 
	vec3 specColor = texture2D(material.specular, wTexCoord).rgb;
    vec3 viewDir = normalize(viewPos - wPos.xyz);
    vec3 reflectDir = reflect(-lightDir, pixelNorm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = spec * specColor * light.specular;
	
	vec3 emission = calculate_emission();

    vec3 result = ambient + diffuse + specular + emission;
    fragColor = vec4(result, 1.0);
}