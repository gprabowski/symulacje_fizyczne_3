#version 420 core

layout(location = 0) in vec3 pos;

out vec3 wpos;
out vec3 normal;
out vec3 view;

uniform mat4 m_view;
uniform mat4 m_proj;
uniform mat4 m_inv_view;

uniform vec3 points[64];

vec3 p(int i, int j, int k){
    return points[i*16+j*4+k];
}

vec3 CubicDeCasteljau(float t, vec3 pos[4]) {
    float t1 = (1.0f - t);
    vec3 p0 = pos[0] * t + pos[1] * t1;
    vec3 p1 = pos[1] * t + pos[2] * t1;
    vec3 p2 = pos[2] * t + pos[3] * t1;
    vec3 p3 = p0 * t + p1 * t1;
    vec3 p4 = p1 * t + p2 * t1;
    return p3 * t + p4 * t1;
}

vec3 bezier(float u, float v, float w){
    vec3 bv0 = CubicDeCasteljau(w, vec3[4](
                                CubicDeCasteljau(v, vec3[4](p(0,0,0),p(0,1,0),p(0,2,0),p(0,3,0))),
                                CubicDeCasteljau(v, vec3[4](p(0,0,1),p(0,1,1),p(0,2,1),p(0,3,1))),
                                CubicDeCasteljau(v, vec3[4](p(0,0,2),p(0,1,2),p(0,2,2),p(0,3,2))),
                                CubicDeCasteljau(v, vec3[4](p(0,0,3),p(0,1,3),p(0,2,3),p(0,3,3)))
                                ));
    vec3 bv1 = CubicDeCasteljau(w, vec3[4](
                                CubicDeCasteljau(v, vec3[4](p(1,0,0),p(1,1,0),p(1,2,0),p(1,3,0))),
                                CubicDeCasteljau(v, vec3[4](p(1,0,1),p(1,1,1),p(1,2,1),p(1,3,1))),
                                CubicDeCasteljau(v, vec3[4](p(1,0,2),p(1,1,2),p(1,2,2),p(1,3,2))),
                                CubicDeCasteljau(v, vec3[4](p(1,0,3),p(1,1,3),p(1,2,3),p(1,3,3)))
                                ));
    vec3 bv2 = CubicDeCasteljau(w, vec3[4](
                                CubicDeCasteljau(v, vec3[4](p(2,0,0),p(2,1,0),p(2,2,0),p(2,3,0))),
                                CubicDeCasteljau(v, vec3[4](p(2,0,1),p(2,1,1),p(2,2,1),p(2,3,1))),
                                CubicDeCasteljau(v, vec3[4](p(2,0,2),p(2,1,2),p(2,2,2),p(2,3,2))),
                                CubicDeCasteljau(v, vec3[4](p(2,0,3),p(2,1,3),p(2,2,3),p(2,3,3)))
                                ));
    vec3 bv3 = CubicDeCasteljau(w, vec3[4](
                                CubicDeCasteljau(v, vec3[4](p(3,0,0),p(3,1,0),p(3,2,0),p(3,3,0))),
                                CubicDeCasteljau(v, vec3[4](p(3,0,1),p(3,1,1),p(3,2,1),p(3,3,1))),
                                CubicDeCasteljau(v, vec3[4](p(3,0,2),p(3,1,2),p(3,2,2),p(3,3,2))),
                                CubicDeCasteljau(v, vec3[4](p(3,0,3),p(3,1,3),p(3,2,3),p(3,3,3)))
                                ));
    return CubicDeCasteljau(u, vec3[4](bv0,bv1,bv2,bv3));
}

void main(){

    wpos = bezier(pos.x, pos.y, pos.z);
    vec3 pos2 = ((pos-vec3(0.5f, 0.5f, 0.5f)) * 0.9f) + vec3(0.5f, 0.5f, 0.5f);
    vec3 wpos2 = bezier(pos2.x, pos2.y, pos2.z);

    normal = normalize(wpos-wpos2);

    gl_Position = m_proj * m_view * vec4(wpos, 1.0);
    //wpos = (m_trans * vec4(pos, 1.0)).xyz;
    vec3 cam = (m_inv_view * vec4(0.0f, 0.0f, 0.0f, 1.0f)).xyz;
    view = normalize(cam - wpos);
}
