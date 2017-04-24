#version 120

attribute vec3 b_vertex;
attribute vec2 b_text;

uniform mat4 m_projection;

varying vec2 v_texcoord;

void main() {
    v_texcoord = b_text;
    gl_Position = m_projection * vec4(b_vertex, 1);
}
