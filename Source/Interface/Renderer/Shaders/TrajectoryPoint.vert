attribute vec4 position;
attribute vec4 colour;

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;

void main()
{
    // gl_PointSize = 12.5  - (distance(position, vec4(cameraPosition, 0.0)) / 12.5); //todo 
    
    gl_PointSize = 15.0;
    vec4 adjustedPosition = position;
    if (adjustedPosition == vec4(0.0, 0.0, 0.0, 0.0))
    {
        adjustedPosition = vec4(0.0, 0.0, 40.0, 0.0);
    }
    adjustedPosition.z = (position.z /* 0.5 + 1.0*/) * 0.3 + 0.21;
    
    gl_Position = projectionMatrix * viewMatrix * adjustedPosition;
}