in float depth;
in vec3 normal;
in vec3 fragmentPosition;

uniform vec3 lightPosition;// = vec3(10.0, 1.0, 3.0);
uniform vec3 color;
float ambient = 0.4;

void main()
{
    vec3 lightDirection = normalize(lightPosition - fragmentPosition);
    float diffuseScalar = max(dot(normal, lightDirection), 0.0);
    gl_FragColor = (vec4(color.r / 255.0, color.g / 255.0, color.b / 255.0, 1.0) * (diffuseScalar + ambient));
}