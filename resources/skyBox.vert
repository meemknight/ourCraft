#version 330 core
//https://community.khronos.org/t/rendering-a-skybox-after-drawing-post-process-effects-to-a-screen-quad/74002

out vec3 v_vsViewDirection;

uniform mat4 u_invView;
uniform mat4 u_invProj;

void main()
{
    gl_Position = vec4(((gl_VertexID & 1) << 2) - 1, (gl_VertexID & 2) * 2 - 1, 0.0, 1.0);
    v_vsViewDirection = mat3(u_invView) * (u_invProj * gl_Position).xyz;
}