vec4 effect() {
    // return vec4(vec3(pow(texture(u_shadows, v_uv).r, 5.0) * 1000.0), 1.0);
    return texture(u_screen, v_uv);
}
