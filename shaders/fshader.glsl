#version 420 core

in vec3 wpos;
in vec3 view;

uniform vec4 m_color;
uniform bool grid;
uniform bool shading;

out vec4 FragColor;

const vec3 light_pos = vec3(0.0f, 10.0f, 0.0f);

void main()
{
    if (!grid){
        FragColor = m_color;
        if (!shading)
            return;

        vec3 normal = -normalize(cross(dFdx(wpos), dFdy(wpos)));
        float intensity = 0.3f;
        vec3 l = normalize(light_pos - wpos);
        vec3 h = normalize(view + l);
        intensity += 0.3f * clamp(dot(normal, l), 0.0f, 1.0f);
        intensity += 0.7f * pow(clamp(dot(normal, h), 0.0f, 1.0f), 16.0f);
        intensity = min(intensity, 1.0f);
        FragColor = vec4(intensity * FragColor.rgb, m_color.a);
    }else{
        vec2 coord = wpos.xz;
        vec2 derivative = fwidth(coord);
        vec2 grid = abs(fract(coord - 0.5f) - 0.5f) / derivative;
        float line = min(grid.x, grid.y);
        vec4 color = vec4(0.4f, 0.4f, 0.4f, 1.0f - min(line, 1.0f));
        float minz = min(derivative.y, 1.0f);
        float minx = min(derivative.x, 1.0f);
        if(wpos.x > -minx && wpos.x <minx)
            color.z = 1.0f;
        if(wpos.z > -minz && wpos.z <minz)
            color.x = 1.0f;
        FragColor = color;
    }
}
