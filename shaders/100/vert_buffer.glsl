#version 100

precision mediump int;
precision mediump float;

attribute vec3 b_vertex;
attribute vec3 b_normal;
attribute vec4 b_color;

uniform mat4 m_mvp;
uniform mat4 m_mv;
uniform mat4 m_normal;

varying vec4 v_color;
varying vec3 v_normal;
varying vec3 v_position;

void main(void) {
    v_color = b_color;
    //v_normal = b_normal;
    v_normal = mat3(m_normal) * b_normal;
    v_position = vec3(m_normal * vec4(b_vertex, 1.0));
    gl_Position = m_mvp * vec4(b_vertex, 1.0);
}
