#version 120
attribute vec3 glVertexB;
attribute vec4 glColorB;
varying vec4 Vertex_UV;

void main() {
    vec4 vert = vec4(glVertexB, 1);
    Vertex_UV = vert;
    gl_Position = gl_ModelViewProjectionMatrix * vert;
}
