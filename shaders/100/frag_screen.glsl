#version 100

precision mediump int;
precision mediump float;

const float PI = 3.1415926535;
uniform float barrel_power;

uniform sampler2D tex0;
varying vec2 v_texcoord;

vec2 Distort(vec2 p) {
    float theta = atan(p.y, p.x);
    float radius = length(p);
    radius = pow(radius, barrel_power);
    p.x = radius * cos(theta);
    p.y = radius * sin(theta);
    return 0.5 * (p + 1.0);
}

void main() {
    vec2 xy = 2.0 * v_texcoord - 1.0;
    vec2 uv;
    float d = length(xy);
    if (d < 1.0) {
        uv = Distort(xy);
    }
    else {
        uv = v_texcoord;
    }
    gl_FragColor = texture2D(tex0, uv);
}
