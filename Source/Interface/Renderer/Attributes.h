#pragma once

#include <juce_opengl/juce_opengl.h>

struct Vertex
{
    float position[3]; // x, y, z
};
struct Attributes
{
    explicit Attributes (juce::OpenGLShaderProgram& shader)
    {
        position.reset (createAttribute ( shader, "position"));
    }
    void enable()
    {
        if(position.get() != nullptr)
        {
            juce::gl::glVertexAttribPointer (position->attributeID, 
                                             3, 
                                             juce::gl::GL_FLOAT, 
                                             juce::gl::GL_FALSE, 
                                             sizeof(float) * 3, 
                                             nullptr); // no offset since first entry
            
            juce::gl::glEnableVertexAttribArray (position->attributeID);
        }
    }
    void disable()
    {
        if (position.get() != nullptr) 
            juce::gl::glDisableVertexAttribArray (position->attributeID);
    }
    std::unique_ptr<juce::OpenGLShaderProgram::Attribute> position;
private:
    static juce::OpenGLShaderProgram::Attribute* createAttribute (juce::OpenGLShaderProgram& shader, 
                                                                  const char* attributeName)
    {
        if (juce::gl::glGetAttribLocation (shader.getProgramID(), attributeName) < 0)
            return nullptr;
        return new juce::OpenGLShaderProgram::Attribute (shader, attributeName); 
    }
};