#version 330 core
out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube skyboxDay;
uniform samplerCube skyboxNight;

uniform float timesOfDay; // Момент времени в игровых сутках

// Цветовые параметры
uniform vec3 sunColor = vec3(1.0, 0.98, 0.09); // Жёлтый
uniform vec3 dawnColor = vec3(1.0, 0.5, 0.2); // Бледно-оранжевый
uniform vec3 duskColor = vec3(0.8, 0.3, 0.1); // Оранжеватый

// Параметры Солнца
uniform float sunSize = 0.999;
uniform float sunGlowStrength = 0.5;
uniform float sunAlpha = 1.0;

// Параметры переходов
uniform float dayStart = 0.2;
uniform float dayEnd = 0.8;
uniform float dawnStart = 0.15;
uniform float dawnEnd = 0.3;
uniform float duskStart = 0.7;
uniform float duskEnd = 0.85;

const float PI = 3.14159265359;

void main() {
    // Небо
    float dayNightFactor = smoothstep(dayStart, dayStart + 0.1, timesOfDay) * (1.0 - smoothstep(dayEnd - 0.1, dayEnd, timesOfDay));

    float dawnFactor = smoothstep(dawnStart, dawnStart + 0.05, timesOfDay) * (1.0 - smoothstep(dawnEnd - 0.05, dawnEnd, timesOfDay));
    float duskFactor = smoothstep(duskStart, duskStart + 0.05, timesOfDay) * (1.0 - smoothstep(duskEnd - 0.05, duskEnd, timesOfDay));
    float dawnDuskFactor = max(dawnFactor, duskFactor);

    vec4 dayColor = texture(skyboxDay, TexCoords);
    vec4 nightColor = texture(skyboxNight, TexCoords);
    vec3 dawnDuskColor = mix(dawnColor, duskColor, abs(timesOfDay - 0.5) * 2.0);

    vec4 skyColor = mix(nightColor, dayColor, dayNightFactor);
    skyColor.rgb = mix(skyColor.rgb, dawnDuskColor, dawnDuskFactor * 0.3);

    // Солнце
    float sunAngle = timesOfDay * 2.0 * PI;
    vec3 sunDirection = vec3(sin(sunAngle), -cos(sunAngle), 0.0);
    vec3 viewDir = normalize(TexCoords);

    float sunDot = dot(viewDir, sunDirection);
    float sunDisk = smoothstep(sunSize - 0.002, sunSize, sunDot);
    float sunGlow = pow(max(0.0, sunDot), 64.0) * sunGlowStrength;

    vec4 sunFinal = vec4(sunColor * (sunDisk + sunGlow), sunAlpha);

    // Итог
    vec3 finalColor = (sunColor * (sunDisk + sunGlow) * dayNightFactor) + (1.0 - (sunAlpha * (sunDisk + sunGlow) * dayNightFactor)) * skyColor.rgb;

    FragColor = skyColor + sunFinal;
}