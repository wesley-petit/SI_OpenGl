#version 450

out vec4 color;
uniform vec4 customColor;

void main()
{
    color = customColor;
}
