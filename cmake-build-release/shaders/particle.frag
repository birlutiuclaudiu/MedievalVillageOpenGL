#version 330 core

in vec2 TexCoords;
out vec4 color;
uniform sampler2D diffuseTexture;
void main()
{
    vec4 color1 = texture(diffuseTexture, TexCoords);
    color =color1 + vec4(0.3f, 0.0f, 0.0f, 0.0f);

}