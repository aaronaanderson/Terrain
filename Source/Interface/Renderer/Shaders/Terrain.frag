in float depth;
in vec3 normal;
in vec3 fragmentPosition;

uniform ivec4 color;
uniform vec3 lightPosition;
float ambient = 0.4;

void main()
{
    vec3 lightDirection = normalize(lightPosition - fragmentPosition);
    float diffuseScalar = max(dot(normal, lightDirection), 0.0);
    float alpha = color.a / 255.0;
    vec3 rgb = vec3(color.r, color.g, color.b) / 255.0;

    float lighting = diffuseScalar + ambient;
    vec3 litRGB = rgb * lighting;

    gl_FragColor = vec4(litRGB, alpha);
}