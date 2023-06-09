#version 460 core

in vec4 Norm;
in vec3 TexCoord; //v3
out vec4 FragColor;

uniform samplerCube TexCube;

void main(){
    FragColor = texture(TexCube, TexCoord);
}