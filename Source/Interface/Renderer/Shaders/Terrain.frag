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
    gl_FragColor = (vec4(color.r / 255.0, color.g / 255.0, color.b / 255.0, color.a / 255.0) * (diffuseScalar + ambient));
}