#version 450

in vec3 position;
uniform vec3 translate;
uniform mat4 rotate;

void main()
{
    gl_Position = rotate * vec4(position * 0.005 + translate, 1.0);
}
