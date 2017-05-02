#version 120

attribute vec3 b_vertex;
attribute vec3 b_normal;
attribute vec4 b_color;

uniform mat4 m_mvp;

varying vec4 v_color;
varying vec3 v_normal;
varying vec3 v_vertex;

void main(void) {
    v_color = b_color;
    v_normal = b_normal;
    v_vertex = b_vertex;
    gl_Position = m_mvp * vec4(b_vertex, 1.0);
}
