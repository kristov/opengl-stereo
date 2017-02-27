#version 120
attribute vec3 glVertexB;
attribute vec2 glTextB;
varying vec2 Vertex_UV;

void main() {
    vec4 vert = vec4(glVertexB, 1);
    Vertex_UV = glTextB;
    gl_Position = gl_ModelViewProjectionMatrix * vec4(glVertexB, 1);
}
