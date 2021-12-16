#version 420 core

in vec3 wpos;
in vec3 normal;
in vec3 view;

out vec4 FragColor;

const vec3 light_pos = vec3(2.0f, 8.0f, 2.0f);

void main()
{
    vec3 color = vec3(0.1f, 0.1f, 0.1f);
    vec3 l = normalize(light_pos - wpos);
    vec3 h = normalize(view + l);
    color += 0.5*dot(normal, l);
    color += 0.5*pow(dot(normal, h), 30.0f);

    //FragColor = vec4((normal), 1.0f);
    FragColor = vec4(color, 1.0f);
    //FragColor = vec4((view), 1.0f);
}
