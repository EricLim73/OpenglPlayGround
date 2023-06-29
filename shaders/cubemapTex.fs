#version 460 core

in vec4 wPos;
in vec4 wNorm; 
in vec3 wTexCoord; // v3
out vec4 FragColor;

struct Light {
    vec3 position; 
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    vec3 attenuation; 
    vec2 cutoff;
    int lightType;
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
uniform float timeStamp; // for emmision animation effect

vec3 calculateDirectionalLight(vec3 texColor, vec3 specColor, vec3 emission){
    // ambient 
  	vec3 ambient = texColor * light.ambient;

    // diffuse 
    vec3 pixelNorm = normalize(wNorm.xyz); 
    vec3 lightDir = normalize(-light.direction);  
    float diff = max(dot(pixelNorm, lightDir), 0.0);
    vec3 diffuse = diff * texColor * light.diffuse;
    
    // specular 
    vec3 viewDir = normalize(viewPos - wPos.xyz);
    vec3 reflectDir = reflect(-lightDir, pixelNorm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = spec * specColor * light.specular;
    
    // emission
    //vec3 emission = emissionColor;

    vec3 result = (ambient + diffuse + specular + emission);
    
    return result;
}

vec3 calculatePointLight(vec3 texColor, vec3 specColor, vec3 emission){ 
  	vec3 ambient = texColor * light.ambient;

    float dist = length(light.position - wPos.xyz);
    vec3 distPoly = vec3(1.0, dist, dist*dist);
    float attenuation = 1.0 / dot(distPoly, light.attenuation);
    vec3 lightDir = (light.position - wPos.xyz) / dist;

    vec3 pixelNorm = normalize(wNorm.xyz);
    float diff = max(dot(pixelNorm, lightDir), 0.0);
    vec3 diffuse = diff * texColor * light.diffuse;
    
    vec3 viewDir = normalize(viewPos - wPos.xyz);
    vec3 reflectDir = reflect(-lightDir, pixelNorm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = spec * specColor * light.specular;

    //vec3 emission = calculate_emission();
    
    vec3 result = (ambient + diffuse + specular + emission)*attenuation;
    return result;
}

vec3 calculateSpotLight(vec3 texColor, vec3 specColor, vec3 emission){
    // NOTE: for spotLight, "light.direction" and "lightDir" is two
    //       different things. Former is the actual lightsource direction
    //       and the latter represents direction btwn pixel and lightSource
  	vec3 ambient = texColor * light.ambient;
    //vec3 emission = calculate_emission();

    float dist = length(light.position - wPos.xyz);
    vec3 distPoly = vec3(1.0, dist, dist*dist);
    float attenuation = 1.0 / dot(distPoly, light.attenuation);
    vec3 lightDir = (light.position - wPos.xyz) / dist;

    float theta = dot(lightDir, normalize(-light.direction));
    float intensity = clamp((theta - light.cutoff[1]) / (light.cutoff[0] - light.cutoff[1]), 0.0, 1.0);

    vec3 result = ambient + emission;
    // NOTE: "Theta" is angle value represented as cosine
    //        and this goes same for "light.cutoff". So when 
    //        comparing if theta has bigger value, that means
    //        that the angle for theta is actualy smaller than 
    //        light.cutoff
    //        >   if (theta > light.cutoff)
    //        But when implementing it to have softer edge we compare
    //        different stuff. We want to interpolate from certain inner angle
    //        to the edeg and the formula for that is this
    //
    //                                (theta - cos(radian for angle to edge)) 
    //  [divid symbol]       ------------------------------------------------------------ 
    //                  cos(radian for angle of inner circle) - cos(radian for angle to edge)

    if (intensity > 0.0) {
      vec3 pixelNorm = normalize(wNorm.xyz);
      float diff = max(dot(pixelNorm, lightDir), 0.0);
      vec3 diffuse = diff * texColor * light.diffuse;

      vec3 viewDir = normalize(viewPos - wPos.xyz);
      vec3 reflectDir = reflect(-lightDir, pixelNorm);
      float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
      vec3 specular = spec * specColor * light.specular;

      result += (diffuse + specular) * intensity;
    }
    result *= attenuation;
    return result;
}

void main() {
  vec3 diffuseColor = texture(material.diffuse, wTexCoord).rgb;
  vec3 specColor = texture(material.specular, wTexCoord).rgb;
	
  // For now keep it this way
	vec3 emissionColor = vec3(0.0);
  

  vec3 result;
  if (light.lightType == 0)
	  result = calculateDirectionalLight(diffuseColor, specColor, emissionColor);
  if (light.lightType == 1)
	  result = calculatePointLight(diffuseColor, specColor, emissionColor);
  if (light.lightType == 2)
	  result = calculateSpotLight(diffuseColor, specColor, emissionColor);
  
  
  FragColor = vec4(result, 1.0);
}