#version 450

in vec3 position;
uniform float scale;
uniform vec3 translate;
uniform mat4 rotate;

void main()
{
    gl_Position = vec4(position * scale, 1.0) * rotate + vec4(translate, 1.0);
}
