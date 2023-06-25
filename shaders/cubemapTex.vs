#version 460 core

layout (location = 0) in vec4 aPos;
layout (location = 1) in vec4 aNorm; 
layout (location = 2) in vec3 aTexCoord; //v3

out vec4 wPos;
out vec4 wNorm;
out vec3 wTexCoord; // v3

uniform mat4 model; 
uniform mat4 ViewProj;

void main()
{
    wTexCoord = vec3(aPos.xyz);
	wPos = model * aPos; // position in world frame  
	wNorm = transpose(inverse(model)) * aNorm; // normal vector in world frame
    gl_Position = ViewProj * model * aPos;
}