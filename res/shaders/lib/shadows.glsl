#ifndef SHADOWS_GLSL_
#define SHADOWS_GLSL_

float calc_shadow() {
    float shadow = 1.0;
    if (u_enableShadows) {
        vec4 mpos = u_shadowsMatrix * vec4(a_modelpos.xyz + a_realnormal * 0.08, 1.0);
        vec3 projCoords = mpos.xyz / mpos.w;
        projCoords = projCoords * 0.5 + 0.5;
        projCoords.z -= 0.0001;

        shadow = 0.0;

        if (dot(a_realnormal, u_sunDir) < 0.0) {
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
            shadow = shadow * 0.5 + 0.5;
        } else {
            shadow = 0.5;
        }
    }
    return shadow;
}

#endif // SHADOWS_GLSL_
