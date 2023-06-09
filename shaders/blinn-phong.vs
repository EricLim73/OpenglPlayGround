#version 460

layout (location = 0) in vec4 aPos;
layout (location = 1) in vec4 aNorm; 

out vec4 wPos;
out vec4 wNorm;  

uniform mat4 model; 
uniform mat4 ViewProj;
 

void main(void) {
	wPos = model * aPos; // position in world frame  
	wNorm = transpose(inverse(model)) * aNorm; // normal vector in world frame
	gl_Position = ViewProj* wPos;
}



