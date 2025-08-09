#version 330 core

// Исходящий цвет фрагмента
out vec4 FragColor;

// Входные переменные из вершинного шейдера
in vec3 color; // Цвет вершины (не используется в этом шейдере)
in vec2 texCoord; // Текстурные координаты
in float texID; // Идентификатор текстуры (определяет какую текстуру использовать)
in vec3 Normal; // Нормаль поверхности
in vec3 currPos; // Текущая позиция фрагмента в мировых координатах

// Текстуры
uniform sampler2D tex0;
uniform sampler2D tex1;

// Параметры освещения
uniform vec4 lightColor; // Цвет света
uniform vec3 lightPos; // Позиция источника света
uniform vec3 camPos;

// Точечное освещение 
vec4 pointLight() {
	vec3 lightVec = lightPos - currPos;
	float dist = length(lightVec); // Расстояние до источника света
	float a = 3.0;
	float b = 0.7;
	float inten = 1.0f / (a * dist * dist + b * dist + 1.0f); // Интенсивность света с учётом затухания (чем дальше от источника, тем слабее свет)

	float ambient = 0.20f; // Фоновое (амбиентное) освещение (минимальная освещенность даже в тенях) 

	// Диффузное освещение (рассеянный свет)
	vec3 normal = normalize(Normal);
	vec3 lightDirection = normalize(lightVec);
	float diffuse = max(dot(normal, lightDirection), 0.0f);

	// Зеркальное освещение (блики)
	float specularLight = 0.50f;
	vec3 viewDirection = normalize(camPos - currPos);
	vec3 reflectionDirection = reflect(-lightDirection, normal);
	float specAmount = pow(max(dot(viewDirection, reflectionDirection), 0.0f), 16);
	float specular = specAmount * specularLight;

	return (texture(tex0, texCoord) * (diffuse * inten + ambient) + texture(tex1, texCoord).r * specular * inten) * lightColor;
}

// Направленное освещение (как от солнца)
vec4 direcLight() {
	float ambient = 0.20f; // Фоновое (амбиентное) освещение (минимальная освещенность даже в тенях) 

	// Диффузное освещение (рассеянный свет)
	vec3 normal = normalize(Normal);
	vec3 lightDirection = normalize(vec3(0.0f, 1.0f, 0.0f));
	float diffuse = max(dot(normal, lightDirection), 0.0f);

	// Зеркальное освещение (блики)
	float specularLight = 0.50f;
	vec3 viewDirection = normalize(camPos - currPos);
	vec3 reflectionDirection = reflect(-lightDirection, normal);
	float specAmount = pow(max(dot(viewDirection, reflectionDirection), 0.0f), 16);
	float specular = specAmount * specularLight;

	return (texture(tex0, texCoord) * (diffuse + ambient) + texture(tex1, texCoord).r * specular) * lightColor;
}

// Прожекторное освещение (как от прожектора или фонаря)
vec4 spotLight() {
	vec3 lightVec = lightPos - currPos;

	// Косинусы углов раскрытия конусов прожектора
	float outerCone = 0.90f;
	float innerCone = 0.95f;

	float ambient = 0.20f; // Фоновое (амбиентное) освещение (минимальная освещенность даже в тенях) 

	// Диффузное освещение (рассеянный свет)
	vec3 normal = normalize(Normal);
	vec3 lightDirection = normalize(lightVec);
	float diffuse = max(dot(normal, lightDirection), 0.0f);

	// Зеркальное освещение (блики)
	float specularLight = 0.50f;
	vec3 viewDirection = normalize(camPos - currPos);
	vec3 reflectionDirection = reflect(-lightDirection, normal);
	float specAmount = pow(max(dot(viewDirection, reflectionDirection), 0.0f), 16);
	float specular = specAmount * specularLight;

	// Расчёт мягких границ прожектора
	float angle = dot(vec3(0.0f, -1.0f, 0.0f), -lightDirection);
	float inten = clamp((angle - outerCone) / (innerCone - outerCone), 0.0f, 1.0f);

	return (texture(tex0, texCoord) * (diffuse * inten + ambient) + texture(tex1, texCoord).r * specular * inten) * lightColor;
}

void main(){
	FragColor = spotLight();
}