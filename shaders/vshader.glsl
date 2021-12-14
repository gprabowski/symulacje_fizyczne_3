#version 420 core
layout(location = 0) in vec3 pos;

out vec3 wpos;
out vec3 view;

uniform mat4 m_trans;
uniform mat4 m_view;
uniform mat4 m_proj;
uniform mat4 m_inv_view;


void main(){
    gl_PointSize = 4.0f;
    gl_Position = m_proj * m_view * m_trans * vec4(pos, 1.0);
    wpos = (m_trans * vec4(pos, 1.0)).xyz;
    vec3 cam = (m_inv_view * vec4(0.0f, 0.0f, 0.0f, 1.0f)).xyz;
    view = normalize(cam - wpos);
}
