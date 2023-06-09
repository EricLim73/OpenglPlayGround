#version 410 core
layout (location = 0) in vec2 vertex_position;
uniform float x_dir;
uniform float y_dir;
out vec2 TexCoord;

void main() {
    gl_Position = vec4( vertex_position.x + x_dir - 0.5, 
                        vertex_position.y + y_dir - 0.5, 
                        0.0, 
                        1.0);
    TexCoord = vec2(vertex_position.x, vertex_position.y);
};