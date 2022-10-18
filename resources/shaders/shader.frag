#version 450

in vec3 originalPosition;
in vec3 originalNormal;

out vec4 color;

uniform vec3 lightPosition;
uniform vec3 lightEmitted;
uniform vec3 albedo;
uniform sampler2D tex;

void main()
{
    // Apply lightning
    vec3 directionToLight = lightPosition - originalPosition;
    float distance = dot(directionToLight, directionToLight);
    vec3 omegaI = normalize(directionToLight);

    vec4 radiance = vec4(lightEmitted / distance * dot(originalNormal, omegaI) * albedo, 1.0);   
    vec4 texture = texture(tex, fract(gl_FragCoord.xy / vec2(500, 500)));

    color = texture * radiance;

    //// Display pixel normal
    //color = vec4(abs(originalNormal), 1.0);
}
