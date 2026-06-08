#version 430 core

layout(std140, binding = 0) uniform CMGL
{
    mat4 matWorldToClip;
    vec4 cameraPos;
} cm;

layout(std140, binding = 1) uniform ROCEL
{
    mat4  model;
    vec4  celRgba;
    float uAlphaCelBorder;
} cbp;

layout(std430, binding = 8) readonly buffer EDGEBUFFER
{
    // Per edge:
    // edges[i*4 + 0] = E0 shared edge start
    // edges[i*4 + 1] = E1 shared edge end
    // edges[i*4 + 2] = OA opposite vertex from triangle A
    // edges[i*4 + 3] = OB opposite vertex from triangle B
    vec4 edges[];
};

void CullVertex()
{
    gl_Position = vec4(2.0, 2.0, 2.0, 1.0);
}

float Cross2D(vec2 a, vec2 b)
{
    return a.x * b.y - a.y * b.x;
}

void main()
{
    int  edgeID = gl_VertexID >> 1;
    bool first  = (gl_VertexID & 1) == 0;

    vec3 E0 = edges[edgeID * 4 + 0].xyz;
    vec3 E1 = edges[edgeID * 4 + 1].xyz;
    vec3 OA = edges[edgeID * 4 + 2].xyz;
    vec3 OB = edges[edgeID * 4 + 3].xyz;

    mat4 mvp = cm.matWorldToClip * cbp.model;

    vec4 A = mvp * vec4(E0, 1.0);
    vec4 B = mvp * vec4(E1, 1.0);
    vec4 C = mvp * vec4(OA, 1.0);
    vec4 D = mvp * vec4(OB, 1.0);

    // Similar to rejecting fully bad projected groups.
    // Keep this simple so we do not divide by zero/negative W.
    if (A.w <= 0.0 || B.w <= 0.0 || C.w <= 0.0 || D.w <= 0.0)
    {
        CullVertex();
        return;
    }

    // VU-style: do silhouette test after projection divide.
    vec3 a = A.xyz / A.w;
    vec3 b = B.xyz / B.w;
    vec3 c = C.xyz / C.w;
    vec3 d = D.xyz / D.w;

    // Triangles:
    // tri0 = OA, E0, E1
    // tri1 = E0, E1, OB
    //
    // Shared border is E0 -> E1.
    float cross0 = Cross2D(b.xy - a.xy, c.xy - b.xy);
    float cross1 = Cross2D(d.xy - b.xy, b.xy - a.xy);

    bool drawEdge = (cross0 * cross1) <= 0.0;

    if (!drawEdge)
    {
        CullVertex();
        return;
    }

    gl_Position = first ? A : B;
}