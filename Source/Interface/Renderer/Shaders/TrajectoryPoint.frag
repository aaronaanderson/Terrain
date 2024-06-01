uniform vec3 color;

void main()
{
    float d = distance(gl_PointCoord, vec2(0.5, 0.5)) * 2;
    // float transparency = 1.0 - sqrt(sqrt(sqrt(d)));
    float transparency = 1.0 - pow(d, 0.1);
    gl_FragColor = vec4(color.r / 255.0, color.g / 255.0, color.b / 255.0, transparency * 0.1);
}