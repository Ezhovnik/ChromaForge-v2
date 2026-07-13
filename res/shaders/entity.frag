layout (location = 0) out vec4 f_color;
layout (location = 1) out vec4 f_position;
layout (location = 2) out vec4 f_normal;

in float a_distance;
in float a_fog;
in vec2 a_texCoord;
in vec3 a_dir;
in vec3 a_normal;
in vec3 a_position;
in vec3 a_realnormal;
in vec4 a_color;
in vec4 a_modelpos;

uniform sampler2D u_texture0;
uniform samplerCube u_skybox;
uniform vec3 u_fogColor;
uniform float u_fogFactor;
uniform float u_fogCurve;
uniform bool u_alphaClip;

uniform vec3 u_sunDir;

#include <shadows>

void main() {
    float shadow = calc_shadow(a_modelpos, a_realnormal, a_distance);
    vec3 fogColor = texture(u_skybox, a_dir).rgb;
    vec4 tex_color = texture(u_texture0, a_texCoord);
    float alpha = a_color.a * tex_color.a;
    if (alpha < (u_alphaClip ? 0.5f : 0.15f)) {
        discard;
    }
    f_color = a_color * tex_color * shadow;
    f_color = mix(f_color, vec4(fogColor, 1.0), a_fog);
    f_color.a = alpha;
    f_position = vec4(a_position, 1.0);
    f_normal = vec4(a_normal, 1.0);
}
