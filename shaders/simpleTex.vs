#version 460 core

layout (location = 0) in vec4 aPos;
layout (location = 1) in vec4 aNorm; 
layout (location = 2) in vec3 aTexCoord;

out vec4 Norm;
out vec2 TexCoord;

uniform mat4 model; 
uniform mat4 ViewProj;

void main()
{
    gl_Position = ViewProj * model * aPos;
    TexCoord = vec2(aTexCoord.x, aTexCoord.y);
    Norm = aNorm;
}