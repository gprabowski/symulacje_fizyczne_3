#version 420 core

in vec3 wpos;
in vec3 normal;
in vec3 view;

out vec4 FragColor;

const vec3 light_pos = vec3(0.0f, 10.0f, 0.0f);

void main()
{
    FragColor = vec4(0.2f, 0.8f, 0.2f, 1.0f);

    //vec3 normal = -normalize(cross(dFdx(wpos), dFdy(wpos)));
    float intensity = 0.3f;
    vec3 l = normalize(light_pos - wpos);
    vec3 h = normalize(view + l);
    intensity += 0.3f * clamp(dot(normal, l), 0.0f, 1.0f);
    intensity += 0.7f * pow(clamp(dot(normal, h), 0.0f, 1.0f), 16.0f);
    intensity = min(intensity, 1.0f);
    FragColor = vec4(intensity * FragColor.rgb, 1.0f);
    //FragColor = vec4(normal, 1.0f);
}
