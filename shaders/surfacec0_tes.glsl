#version 420 core

//layout(isolines) in;
layout(quads,fractional_even_spacing) in;

uniform mat4 m_view;
uniform mat4 m_inv_view;
uniform mat4 m_proj;

out vec3 wpos;
out vec3 normal;
out vec3 view;


patch in float sgm;

vec4 p(int i, int j){
    return gl_in[i*4+j].gl_Position;
}

void main(){
    float u = gl_TessCoord.x;
    float v = gl_TessCoord.y;
    float up = 1.0f - u;
    float vp = 1.0f - v;

    float bu0 = up * up * up;
    float bu1 = 3.0f * u * up * up;
    float bu2 = 3.0f * u * u * up;
    float bu3 = u * u * u;
    float bv0 = vp * vp * vp;
    float bv1 = 3.0f * v * vp * vp;
    float bv2 = 3.0f * v * v * vp;
    float bv3 = v * v * v;

gl_Position =
            ( bu0 * ( bv0*p(0,0) + bv1*p(0,1) + bv2*p(0,2) + bv3*p(0,3) )
            + bu1 * ( bv0*p(1,0) + bv1*p(1,1) + bv2*p(1,2) + bv3*p(1,3) )
            + bu2 * ( bv0*p(2,0) + bv1*p(2,1) + bv2*p(2,2) + bv3*p(2,3) )
            + bu3 * ( bv0*p(3,0) + bv1*p(3,1) + bv2*p(3,2) + bv3*p(3,3) ));

    vec4 du,dv;
{
    float u0 = -3.0f * up * up;
    float u1 = 3.0f * (3.0f * u * u - 4.0f * u + 1.0f);
    float u2 = 3.0f * u * (2.0f - 3.0f * u);
    float u3 = 3.0f * u * u;
    float v0 = vp * vp * vp;
    float v1 = 3.0f * v * vp * vp;
    float v2 = 3.0f * v * v * vp;
    float v3 = v * v * v;
    du =       ( u0 * (v0 * p(0,0) + v1 * p(0,1) + v2 * p(0,2) + v3 * p(0,3))
               + u1 * (v0 * p(1,0) + v1 * p(1,1) + v2 * p(1,2) + v3 * p(1,3))
               + u2 * (v0 * p(2,0) + v1 * p(2,1) + v2 * p(2,2) + v3 * p(2,3))
               + u3 * (v0 * p(3,0) + v1 * p(3,1) + v2 * p(3,2) + v3 * p(3,3)));
}
{
    float v0 = -3.0f * vp * vp;
    float v1 = 3.0f * (3.0f * v * v - 4.0f * v + 1.0f);
    float v2 = 3.0f * v * (2.0f - 3.0f * v);
    float v3 = 3.0f * v * v;
    float u0 = up * up * up;
    float u1 = 3.0f * u * up * up;
    float u2 = 3.0f * u * u * up;
    float u3 = u * u * u;
    dv =       ( u0 * (v0 * p(0,0) + v1 * p(0,1) + v2 * p(0,2) + v3 * p(0,3))
               + u1 * (v0 * p(1,0) + v1 * p(1,1) + v2 * p(1,2) + v3 * p(1,3))
               + u2 * (v0 * p(2,0) + v1 * p(2,1) + v2 * p(2,2) + v3 * p(2,3))
               + u3 * (v0 * p(3,0) + v1 * p(3,1) + v2 * p(3,2) + v3 * p(3,3)));
}
    normal = normalize(cross(du.xyz, dv.xyz));
    wpos = gl_Position.xyz;
    vec3 cam = (m_inv_view * vec4(0.0f, 0.0f, 0.0f, 1.0f)).xyz;
    view = normalize(cam - wpos);
    gl_Position = m_proj * m_view * gl_Position;

}
