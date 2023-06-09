#version 460 core

in vec4 Norm;
in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D Tex;

void main(){
    FragColor = texture(Tex, TexCoord);
}