#version 330 core
out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube skyboxDay;
uniform samplerCube skyboxNight;

uniform float timesOfDay; // Момент времени в игровых сутках

// Цветовые параметры
uniform vec3 dawnColor = vec3(1.0, 0.5, 0.2); // Бледно-оранжевый
uniform vec3 duskColor = vec3(0.8, 0.3, 0.1); // Оранжеватый

// Параметры горизонта
uniform float horizonStart = 0.1;
uniform float horizonFalloff = 2.0;
uniform float horizonIntensity = 0.5;

// Параметры Солнца
uniform vec3 sunColor;
uniform vec3 sunPosition;
uniform float sunSize;
uniform float sunGlowStrength;
uniform float sunAlpha;

// Параметры Луны
uniform vec3 moonPosition;
uniform vec3 moonDiskColor;
uniform vec3 moonGlowColor;
uniform float moonSize;
uniform float moonGlowStrength;
uniform float moonAlpha;

// Параметры переходов
uniform float dayStart = 0.2;
uniform float dayEnd = 0.8;
uniform float dawnStart = 0.2;
uniform float dawnEnd = 0.3;
uniform float duskStart = 0.7;
uniform float duskEnd = 0.8;

void main() {
    // Небо
    float dayNightFactor = smoothstep(dayStart, dayStart + 0.1, timesOfDay) * (1.0 - smoothstep(dayEnd - 0.1, dayEnd, timesOfDay));

    float dawnFactor = smoothstep(dawnStart, dawnStart + 0.05, timesOfDay) * (1.0 - smoothstep(dawnEnd - 0.05, dawnEnd, timesOfDay));
    float duskFactor = smoothstep(duskStart, duskStart + 0.05, timesOfDay) * (1.0 - smoothstep(duskEnd - 0.05, duskEnd, timesOfDay));
    float dawnDuskFactor = max(dawnFactor, duskFactor);

    vec4 dayColor = texture(skyboxDay, TexCoords);
    vec4 nightColor = texture(skyboxNight, TexCoords);
    vec3 dawnDuskColor = mix(dawnColor, duskColor, abs(timesOfDay - 0.5) * 2.0);

    vec4 skyFinal = mix(nightColor, dayColor, dayNightFactor);

    float horizonHeight = abs(normalize(TexCoords).y);
    float horizonMask = 1.0 - smoothstep(horizonStart, 1.0, horizonHeight);
    horizonMask = pow(horizonMask, horizonFalloff);

    skyFinal.rgb = mix(skyFinal.rgb, dawnDuskColor, dawnDuskFactor * horizonMask * horizonIntensity);

    vec3 viewDir = normalize(TexCoords);

    // Солнце
    vec3 sunDir = normalize(sunPosition);

    float sunDot = dot(viewDir, sunDir);
    float sunDisk = smoothstep(sunSize - 0.002, sunSize, sunDot);
    float sunGlow = pow(max(0.0, sunDot), 64.0) * sunGlowStrength;

    vec4 sunFinal = vec4(sunColor * (sunDisk + sunGlow) * dayNightFactor, sunAlpha);

    // Луна
    vec3 moonDir = normalize(moonPosition);

    float moonDot = dot(viewDir, moonDir);
    float moonDisk = smoothstep(moonSize - 0.002, moonSize, moonDot);
    float moonGlow = pow(max(0.0, moonDot), 64.0) * moonGlowStrength;

    vec3 moonColorFinal = moonDiskColor * moonDisk + moonGlowColor * moonGlow;
    vec4 moonFinal = vec4(moonColorFinal * (1 - dayNightFactor), moonAlpha);

    // Итог
    FragColor = skyFinal + sunFinal + moonFinal;
}