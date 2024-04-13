#version 330 core
layout (location = 0) in vec3 a_Pos;
layout (location = 1) in vec2 a_TexCoords;

noperspective out vec2 v_texCoords;

void main()
{
    v_texCoords = a_TexCoords;
    gl_Position = vec4(a_Pos, 1.0);
}