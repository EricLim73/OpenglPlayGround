#version 410 core

uniform sampler2D Tex;
uniform float uv_x;
uniform float uv_y;
uniform float nx_frames;
uniform float ny_frames;

in vec2 TexCoord;
out vec4 frag_color;

void main(){
    float x = 1.0 / nx_frames;
    float y = 1.0 / ny_frames;
    frag_color = texture(Tex, 
                         vec2(TexCoord.x * x, TexCoord.y * y) + 
                         vec2(x * uv_x, y * uv_y));
    if (frag_color.a <= 0.0) {
        discard;
    }
};