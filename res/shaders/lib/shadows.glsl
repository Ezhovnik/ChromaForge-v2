#ifndef SHADOWS_GLSL_
#define SHADOWS_GLSL_

uniform sampler2DShadow u_shadows;
uniform sampler2DShadow u_wideShadows;
uniform int u_shadowsRes;
uniform bool u_blurShadows;
uniform mat4 u_shadowsMatrix;
uniform mat4 u_wideShadowsMatrix;

float calc_shadow() {
    float shadow = 1.0;
    if (u_enableShadows) {
        vec4 mpos = u_shadowsMatrix * vec4(a_modelpos.xyz + a_realnormal * 0.04, 1.0);
        vec3 projCoords = mpos.xyz / mpos.w;
        projCoords = projCoords * 0.5 + 0.5;
        projCoords.z -= 0.0001;

        vec4 wmpos = u_wideShadowsMatrix * vec4(a_modelpos.xyz + a_realnormal * 0.2, 1.0);
        vec3 wprojCoords = wmpos.xyz / wmpos.w;
        wprojCoords = wprojCoords * 0.5 + 0.5;
        wprojCoords.z -= 0.0001;

        shadow = 0.0;

        if (dot(a_realnormal, u_sunDir) < 0.0) {
            if (u_blurShadows) {
                const vec3 offsets[4] = vec3[4](
                    vec3(0.5, 0.5, 0.0),
                    vec3(-0.5, 0.5, 0.0),
                    vec3(0.5, -0.5, 0.0),
                    vec3(-0.5, -0.5, 0.0)
                );
                for (int i = 0; i < 4; i++) {
                    shadow += texture(u_shadows, projCoords.xyz + offsets[i] / u_shadowsRes);
                }
                shadow /= 4;
            } else {
                if (a_distance > 32.0) {
                    shadow = texture(u_wideShadows, wprojCoords.xyz);
                } else {
                    shadow = texture(u_shadows, projCoords.xyz);
                }
            }
            shadow = shadow * 0.5 + 0.5;
        } else {
            shadow = 0.5;
        }
    }
    return shadow;
}

#endif // SHADOWS_GLSL_
