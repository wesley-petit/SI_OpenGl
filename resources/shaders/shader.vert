#version 450

in vec3 position;
in vec3 normal;

out vec3 originalPosition;
out vec3 originalNormal;

uniform vec3 translate;
uniform mat4 transform;

void main()
{
    originalPosition = position;
    originalNormal = normal;
    gl_Position = vec4(position, 1.0) * transform + vec4(translate, 1.0);
}
