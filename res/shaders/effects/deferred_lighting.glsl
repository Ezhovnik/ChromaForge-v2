#include <shadows>
#include <fog>

vec4 effect() {
    float ssao = 0.0;
    vec4 pos = texture(u_position, v_uv);
    float z = pos.z;
    for (int y = -2; y <= 2; y++) {
        for (int x = -2; x <= 2; x++) {
            vec2 offset = vec2(x, y) / u_screenSize;
            ssao += texture(u_ssao, v_uv + offset * 2.0).r;
        }
    }
    ssao /= 24.0;
    vec4 modelpos = u_inverseView * pos;
    vec3 normal = transpose(mat3(u_view)) * texture(u_normal, v_uv).xyz;
    vec3 dir = modelpos.xyz - u_cameraPos;
#ifdef ENABLE_SHADOWS
    ssao *= calc_shadow(modelpos, normal, length(pos));
#endif
    vec3 fogColor = texture(u_skybox, dir).rgb;
    float fog = calc_fog(length(u_view * vec4((modelpos.xyz - u_cameraPos) * FOG_POS_SCALE, 0.0)) / 256.0);
    return vec4(mix(texture(u_screen, v_uv).rgb * mix(1.0, ssao, 1.0), fogColor, fog), 1.0);
}
