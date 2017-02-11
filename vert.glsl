#version 120
uniform float barrel_power;

vec4 Distort(vec4 p)
{
    vec2 v = p.xy / p.w;
    // Convert to polar coords:
    float radius = length(v);
    if (radius > 0)
    {
      float theta = atan(v.y,v.x);
      
      // Distort:
      radius = pow(radius, barrel_power);

      // Convert back to Cartesian:
      v.x = radius * cos(theta);
      v.y = radius * sin(theta);
      p.xy = v.xy * p.w;
    }
    return p;
}

void main()
{
    gl_FrontColor = gl_Color;
    vec4 P = gl_ModelViewProjectionMatrix * gl_Vertex;
    gl_Position = Distort(P);
}
