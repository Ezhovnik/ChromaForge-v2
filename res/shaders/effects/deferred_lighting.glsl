vec4 effect() {
    float ssao = 0.0;
    float z = texture(u_position, v_uv).z;
    for (int y = -2; y <= 2; y++) {
        for (int x = -2; x <= 2; x++) {
            vec2 offset = vec2(x, y) / u_screenSize;
            if (abs(z - texture(u_position, v_uv + offset * 2.0).z) > 0.05)
                ssao += 1.0;
            else
                ssao += texture(u_ssao, v_uv + offset * 2.0).r;
        }
    }
    ssao /= 24.0;
    return vec4(texture(u_screen, v_uv).rgb * mix(1.0, ssao, 1.0), 1.0);
}
