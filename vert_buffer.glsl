#version 120

attribute vec3 glVertexB;
attribute vec3 glNormalB;
attribute vec4 glColorB;

varying vec3 N;
varying vec3 v;

void main(void) {
    gl_FrontColor = glColorB;
    v = vec3(gl_ModelViewMatrix * vec4(glVertexB, 1));
    N = normalize(gl_NormalMatrix * glNormalB);
    gl_Position = gl_ModelViewProjectionMatrix * vec4(glVertexB, 1);
}
