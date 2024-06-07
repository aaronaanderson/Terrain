attribute vec3 position;

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;

uniform int terrainIndex;

uniform float a;
uniform float b;
uniform float c;
uniform float d;
uniform float saturation;

out float depth;
out vec3 normal;
out vec3 fragmentPosition;

float pi = 3.14159265359;

float saturate (float signal, float scale)
{
    return tanh(signal * scale * 1.31303528551);
}

float calculateDepth (int index, vec2 p)
{
    float outputValue = 0.0;
    switch (index)
    {
        case 0:
            outputValue = sin(p.x * 6.0 * (a + 0.5)) * sin(p.y * 6.0 * (b + 0.5));
        break;
        case 1:
            outputValue = sin((p.x * pi * 2.0) * (p.x * 3.0 * a) + (b * pi * 2.0)) * sin((p.y * pi * 2.0) * (p.y * 3.0 * a) + (b * -pi * 2));       
        break;
        case 2: 
            outputValue = cos(distance(vec2 (p.x, p.y), vec2(0.0, 0.0)) * pi * 2.0 * (a * 5.0 + 1.0) + (b * pi * 2.0)); 
        break;
        case 3:
        {
            float c = a * 14.0 + 1.0;
            outputValue =  (1.0 - (p.x * p.y)) * cos(c * (1.0 - p.x * p.y));
        }
        break;
        case 4:
        {
            float c = a * 0.5 + 0.25;
            float d = b * 16.0 + 4.0;
            outputValue = c * p.x * cos((1.0 - c) * d * pi * p.x * p.y)  +  (1.0 - c) * p.y * cos(c * d * pi * p.x * p.y);
        }
        break;
        default:
            outputValue = 0.0;
    }
    outputValue = saturate (outputValue, saturation);
    return outputValue;
}

// https://stackoverflow.com/questions/13983189/opengl-how-to-calculate-normals-in-a-terrain-height-grid
vec3 calculateNormal (int index, vec2 p)
{
    vec3 offset = vec3(0.05, 0.05, 0.0);
    float hL = calculateDepth (index, p - offset.xz);
    float hR = calculateDepth (index, p + offset.xz);
    float hD = calculateDepth (index, p - offset.zy);
    float hU = calculateDepth (index, p + offset.zy);

    vec3 n;
    n.x = hL - hR;
    n.y = hD - hU;
    n.z = 2.0; // no idea why 2 hmmmm
    n = normalize (n);
    return n;
}

void main()
{
    depth = calculateDepth (terrainIndex, vec2(position.x, position.y));
    vec4 adjustedPosition = vec4(position.x, position.y, depth * 0.3, 1.0);
    depth = 1.0 - (depth * 0.5 + 0.5); // invert depth
    gl_Position = projectionMatrix * viewMatrix * adjustedPosition;
    fragmentPosition = vec3(adjustedPosition.xyz); // not sure about this
    normal = calculateNormal (terrainIndex, vec2(position.xy));
}