#version 420 core

layout(vertices = 16) out;

uniform mat4 m_view;
uniform mat4 m_inv_view;
uniform mat4 m_proj;

uniform int segments_in;
uniform int segments_out;

void main(){
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;

    gl_TessLevelOuter[0] = segments_out;
    gl_TessLevelOuter[1] = segments_out;
    gl_TessLevelOuter[2] = segments_out;
    gl_TessLevelOuter[3] = segments_out;

    gl_TessLevelInner[0] = segments_in;
    gl_TessLevelInner[1] = segments_in;
}
