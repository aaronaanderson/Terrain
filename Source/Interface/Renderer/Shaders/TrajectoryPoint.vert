attribute vec4 position;
attribute vec4 colour;

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;

void main()
{
    // gl_PointSize = 12.5  - (distance(position, vec4(cameraPosition, 0.0)) / 12.5); //todo 
    
    gl_PointSize = 15.0;
    vec4 adjustedPosition = position;
    if (length (adjustedPosition) < 1.000001) // if point is exactly center
    {
        adjustedPosition = vec4(0.0, 0.0, 400.0, 0.0);
    }
    adjustedPosition.z = (position.z /* 0.5 + 1.0*/) * 0.3 + 0.11;
    
    gl_Position = projectionMatrix * viewMatrix * adjustedPosition;
}