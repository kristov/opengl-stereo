#version 120

uniform sampler2D tex0;
varying vec4 Vertex_UV;
const float PI = 3.1415926535;
//uniform float BarrelPower;

vec2 Distort(vec2 p) {
    float theta  = atan(p.y, p.x);
    float radius = length(p);
    radius = pow(radius, 0.6);
    p.x = radius * cos(theta);
    p.y = radius * sin(theta);
    return 0.5 * (p + 1.0);
}

void main() {
    vec2 xy = 2.0 * Vertex_UV.xy - 1.0;
    vec2 uv;
    float d = length(xy);
    if (d < 1.0) {
        uv = Distort(xy);
    }
    else {
        uv = Vertex_UV.xy;
    }
    //vec4 c = texture2D(tex0, uv);
    vec4 c = texture2D(tex0, Vertex_UV.xy);
    gl_FragColor = c;
}
