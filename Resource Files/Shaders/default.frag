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
uniform sampler2D tex2;

// Параметры освещения
uniform vec4 lightColor; // Цвет света
uniform vec3 lightPos; // Позиция источника света
uniform vec3 camPos;

void main()
{
   float ambient = 0.20f;

	// diffuse lighting
	vec3 normal = normalize(Normal);
	vec3 lightDirection = normalize(lightPos - currPos);
	float diffuse = max(dot(normal, lightDirection), 0.0f);

	// specular lighting
	float specularLight = 0.50f;
	vec3 viewDirection = normalize(camPos - currPos);
	vec3 reflectionDirection = reflect(-lightDirection, normal);
	float specAmount = pow(max(dot(viewDirection, reflectionDirection), 0.0f), 8);
	float specular = specAmount * specularLight;

   if (texID == 2.0) {
      FragColor = texture(tex2, texCoord) * lightColor * (diffuse + ambient + specular); 
   } else if (texID == 1.0) {
      FragColor = texture(tex1, texCoord) * lightColor * (diffuse + ambient + specular); 
   } else {
      FragColor = texture(tex0, texCoord) * lightColor * (diffuse + ambient + specular); 
   }
}