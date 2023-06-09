#version 460 core

layout (location = 0) in vec4 aPos;
layout (location = 1) in vec4 aNorm;
layout (location = 2) in vec4 aColor;
layout (location = 3) in vec3 aTexCoord; //v3

out vec4 Norm;
out vec3 TexCoord; // v3

uniform mat4 model; 
uniform mat4 ViewProj;

void main()
{
    gl_Position = ViewProj * model * aPos;
    TexCoord = aTexCoord;
    Norm = aNorm;
}