#version 460

layout (location = 0) in vec4 aPos;
layout (location = 1) in vec4 aNorm; 
layout (location = 2) in vec3 aTexCoord; 

out vec4 wPos;
out vec4 wNorm;  
out vec2 wTexCoord;

uniform mat4 model; 
uniform mat4 ViewProj;


void main(void) {
	wPos = model * aPos; // position in world frame  
	wNorm = transpose(inverse(model)) * aNorm; // normal vector in world frame
	wTexCoord = vec2(aTexCoord.x, aTexCoord.y);
	gl_Position = ViewProj* wPos;
}



