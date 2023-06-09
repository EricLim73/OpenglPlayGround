#version 460

layout (location = 0) in vec4 aPos;
layout (location = 1) in vec4 aNorm;
layout (location = 2) in vec4 aTexCoord;

out vec4 vPos;
out vec4 vNorm; 

uniform mat4 model; 
uniform mat4 ViewProj;
 

void main(void) {
	vPos = model * aPos; // position in view frame  
	vNorm = transpose(inverse(model)) * aNorm; // normal vector in view frame
	gl_Position = ViewProj* vPos;
}



