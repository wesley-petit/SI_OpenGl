#version 450

in vec3 normalGl;
out vec4 color;

//uniform vec4 customColor;

void main()
{
    //color = customColor;
    
    // Display pixel normal
    color = vec4(abs(normalGl), 1.0);
}
