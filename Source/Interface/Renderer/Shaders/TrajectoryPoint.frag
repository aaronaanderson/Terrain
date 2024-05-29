void main()
{
    float d = distance(gl_PointCoord, vec2(0.5, 0.5)) * 2;
    // float transparency = 1.0 - sqrt(sqrt(sqrt(d)));
    float transparency = 1.0 - pow(d, 0.1);
    gl_FragColor = vec4(0.3, 0.4, 0.9, transparency * 0.2);
}