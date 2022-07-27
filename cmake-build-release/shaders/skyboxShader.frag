#version 410 core

in vec3 textureCoordinates;
out vec4 color;

uniform samplerCube skybox;
uniform float intensity;

void main()
{   vec3 coef;
    if(intensity < 0.1)
        coef = vec3(0.1f,0.1f,0.1f);
    else
        coef =vec3(intensity);
    color = texture(skybox, textureCoordinates) * vec4(coef, 1.0f);
}
