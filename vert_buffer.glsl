#version 120
attribute vec3 glVertexB;
attribute vec4 glColorB;

void main()
{
    gl_FrontColor = glColorB;
    gl_Position = gl_ModelViewProjectionMatrix * vec4(glVertexB, 1);
}
