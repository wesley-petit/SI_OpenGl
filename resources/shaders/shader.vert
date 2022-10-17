#version 450

in vec3 position;
in vec3 normal;

out vec3 originalPosition;
out vec3 originalNormal;

uniform mat4 rotate;
uniform vec3 translate;
uniform float scale;

void main()
{
    originalPosition = position;
    originalNormal = normal;
    gl_Position = vec4(position * scale, 1.0) * rotate + vec4(translate, 1.0);
}
