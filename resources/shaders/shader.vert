#version 450

in vec3 position;

void main()
{
    gl_Position = vec4(position * 0.01, 1.0);
}
