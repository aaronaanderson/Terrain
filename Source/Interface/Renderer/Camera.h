#pragma once


// #define GLM_FORCE_CTOR_INIT
// #include <glm/vec3.hpp>
// #include <glm/mat4x4.hpp>
// #include <glm/ext/matrix_transform.hpp>
// #include <glm/ext/matrix_clip_space.hpp>

#include <glm_module/glm_module.hpp>

class Camera
{
public:
    Camera(juce::CriticalSection& m)
      : mutex(m)
    { 
        updateCameraPosition();
    }
    glm::mat4 getProjectionMatrix() const { return projectionMatrix; }
    glm::mat4 getViewMatrix() const { return viewMatrix; }

    void setTargetBounds (const juce::Rectangle<int>& tb)
    {
        targetBounds = tb;
        updateProjectionMatrix();
    }
    void mouseDown (const juce::MouseEvent& e) 
    {
        juce::ignoreUnused(e);
        thetaStart = theta;
        zStart = zRotation;
    }
    void mouseDrag (const juce::MouseEvent& e) 
    { 
        theta = thetaStart + glm::radians (static_cast<float> (e.getDistanceFromDragStartX()) * 0.14f);
        zRotation = zStart + (static_cast<float> (e.getDistanceFromDragStartY()) * 0.005f);
        if (zRotation > juce::MathConstants<float>::halfPi - 0.01f) zRotation = juce::MathConstants<float>::halfPi - 0.01f;
        if (zRotation < 0.5f) zRotation = 0.5f;
        std::cout << zRotation << std::endl;
        updateCameraPosition();
    }
    void mouseWheelMoved (const juce::MouseWheelDetails& wheel) 
    {
        fieldOfView -= wheel.deltaY;
        if (fieldOfView < 45.0f) fieldOfView = 45.0f;
        if (fieldOfView > 120.0f) fieldOfView = 120.0f;
        updateProjectionMatrix();
    }
private:
    juce::CriticalSection& mutex;
    juce::Rectangle<int> targetBounds;

    glm::mat4 projectionMatrix;
    float fieldOfView = 65.0f;
    glm::vec3 cameraPosition;
    glm::vec3 cameraTarget = glm::vec3 (0.0f, 0.0f, 0.0f);
    glm::vec3 cameraFront = glm::vec3 (0.0f, 0.0f, -1.0f);
    glm::vec3 cameraUp = glm::vec3 (0.0f, 0.0f, 1.0f);
    glm::mat4 viewMatrix;

    float theta = 0.0f;
    float thetaStart = 0.0f;

    float zRotation = 0.5f;
    float zStart = 0.0f;

    void updateProjectionMatrix()
    {
        const juce::ScopedLock lock (mutex);
        float ratio = targetBounds.getWidth() / static_cast<float> (targetBounds.getHeight());
        projectionMatrix = glm::perspective (glm::radians (fieldOfView), ratio, 0.1f, 25.0f);
    }
    void updateCameraPosition()
    {
        const juce::ScopedLock lock (mutex);
        cameraPosition = glm::vec3 (std::sin (theta) * 2.0f * std::cos (zRotation), 
                                    std::cos (theta) * 2.0f * std::cos (zRotation), 
                                    std::sin (zRotation) * 2.0f);

        viewMatrix = glm::lookAt(cameraPosition, cameraTarget, cameraUp);
    }
};