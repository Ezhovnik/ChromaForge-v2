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
            for (int x = -1; x <= 1; x++) {
                for (int y = -1; y <= 1; y++) {
                    shadow += texture(u_shadows, projCoords.xyz + vec3(
                        x * (1.0 / u_shadowsRes), 
                        y * (1.0 / u_shadowsRes), 0.0
                    ));
                }
            }
            shadow /= 9;
            shadow = shadow * 0.5 + 0.5;
        } else {
            shadow = 0.5;
        }
    }
    return shadow;
}

#endif // SHADOWS_GLSL_
