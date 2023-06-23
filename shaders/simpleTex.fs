#version 460 core

in vec4 Norm;
in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D Tex1;
uniform sampler2D Tex2;

void main(){
    FragColor = mix(texture(Tex1, TexCoord),texture(Tex2, TexCoord),0.2);
}