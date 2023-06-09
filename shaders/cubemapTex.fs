#version 460 core

in vec4 wPos;
in vec4 wNorm; 
in vec3 TexCoord; // v3
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
	float shininess;
};
uniform Material material;

uniform vec4 viewPos;
uniform samplerCube TexCube;

void main(void) { 
	vec3 N = normalize(wNorm.xyz);
    vec3 L = normalize(light.pos.xyz - wPos.xyz);
	// i had to flip to FragPos - ViewPos instead of the opposite way to get it right
	// or else the specular light goes behind the obj as well.
	// i thought it was bc the viewPos right now is cameraPos and that value is not
	// transformed into WORLD space but didnt change anything when i applied the transformation 
    vec3 V = normalize(wPos.xyz - viewPos.xyz); 
	// dot(R, V) -> dot(N,H) : alpha value (aka shininess value) gets double or higher
	// this is beause the angle between N,H is double the angle of RV (J.Blinn)
	// so when looking at something like when angle is 0, cos() will return 1, 90 will return 0.
	// Then if angle between R,V is between 0~90, the cos value of N,H will always be bigger
	// making it hard to emulate the specular of Phong shading. To counter this, we make the 
	// shininess value much higher to make it simillar
    vec3 H = normalize((L+V));
	//vec3 R = reflect(-L, N);

	float d = length(light.pos.xyz - wPos.xyz);
	float denom = light.att.x + light.att.y * d + light.att.z * d * d;

	vec4 ambient = light.ambient * material.ambient;
	vec4 diffuse = light.diffuse * max(dot(L, N), 0.0) *  (material.diffuse / denom);
	// alpha` >= 2alpha
	vec4 specular = light.specular * pow(max(dot(N, H), 0.0), 3*material.shininess) * (material.specular / denom);
	
	// this was suggested in the comment section from 'learnOpenGL' for weird artifact lighting up.
	// i dont know if this helps bc i didnt had that artifact but for insurance ill put this
	if (diffuse == vec4(0.0, 0.0, 0.0, 0.0))
		specular = vec4(0.0, 0.0, 0.0, 0.0);
    vec4 color = ambient + diffuse + specular;
    vec4 texColor = texture(TexCube, TexCoord);
    FragColor = min(1.5*color*texColor, vec4(1.0,1.0,1.0,1.0));
}