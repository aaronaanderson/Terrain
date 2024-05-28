#pragma once

#include <juce_opengl/juce_opengl.h>

namespace
{
GLenum glCheckError_(const char *file, int line)
{
    GLenum errorCode; GLenum previousError = 0;
    while ((errorCode = juce::gl::glGetError()) != juce::gl::GL_NO_ERROR)
    {
        std::string error;
        switch (errorCode)
        {
            case juce::gl::GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
            case juce::gl::GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
            case juce::gl::GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
            case juce::gl::GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
            case juce::gl::GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
            case juce::gl::GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
            case juce::gl::GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
        }
        std::cout << error << " | " << file << " (" << line << ")" << std::endl;
        
        if(errorCode == previousError) // THIS SHOULDN"T BE NECESSARY - glGetError should remove errors from pile
            break;

        previousError  = errorCode;
    }
    return errorCode;
}
#define ERROR_CHECK() glCheckError_(__FILE__, __LINE__) 
}