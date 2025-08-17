#version 330 core
out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube skyboxDay;
uniform samplerCube skyboxNight;

uniform float timeFactor;

void main() {
    vec4 dayColor = texture(skyboxDay, TexCoords);
    vec4 nightColor = texture(skyboxNight, TexCoords);

    FragColor = mix(nightColor, dayColor, timeFactor);
}