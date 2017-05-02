#version 120

uniform mat4 m_mv;

varying vec4 v_color;
varying vec3 v_normal;
varying vec3 v_vertex;

void main(void) {
    vec3 normal_ms = normalize(vec3(m_mv * vec4(v_normal, 0)));
    vec3 light_ms = vec3(0, 0, 0);
    vec3 vert_ms = vec3(m_mv * vec4(v_vertex, 1.0));
    vec3 stl = light_ms - vert_ms;

    float brightness = dot(normal_ms, stl) / (length(stl) * length(normal_ms));
    brightness = clamp(brightness, 0, 1);

    vec3 d_color = vec3(v_color) * brightness;
    gl_FragColor = vec4(d_color, 1.0);
}
