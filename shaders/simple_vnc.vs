#version 460 core

layout (location = 0) in vec4 aPos;
layout (location = 1) in vec4 aNorm;
layout (location = 2) in vec4 aColor;

out vec4 Color; 

uniform mat4 model; 
uniform mat4 ViewProj;

void main(void) {
	gl_Position = aPos; // transformation
    gl_Position = ViewProj * model * gl_Position;
	Color = aColor;
}
