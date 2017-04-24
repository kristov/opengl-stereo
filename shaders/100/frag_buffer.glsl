#version 100

varying vec4 v_color;
varying vec3 v_normal;
varying vec3 v_position;

void main(void) {
    vec3 light_d = normalize(vec3(0.0, 5.0, 4.0) - v_position);
    vec3 n = normalize(v_normal);
    float diff = max(dot(n, light_d), 0.0);
    vec3 light_c = diff * vec3(1.0, 1.0, 1.0);
    vec3 d_color = vec3(v_color) * light_c;
    gl_FragColor = vec4(d_color, 1.0);
}
