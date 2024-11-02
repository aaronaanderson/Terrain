#pragma once 

#include <juce_opengl/juce_opengl.h>
#include <juce_audio_basics/juce_audio_basics.h>

#include "glUtility.h"
#include "Camera.h"
#include "Attributes.h"

struct TrajectoryUniforms
{
    explicit TrajectoryUniforms (juce::OpenGLShaderProgram& shader)
    { 
        projectionMatrix.reset (createUniform (shader, "projectionMatrix"));
        viewMatrix.reset       (createUniform (shader, "viewMatrix"));
        cameraPosition.reset   (createUniform (shader, "cameraPosition"));
        color.reset            (createUniform (shader, "color"));
    }
    std::unique_ptr<juce::OpenGLShaderProgram::Uniform> projectionMatrix;
    std::unique_ptr<juce::OpenGLShaderProgram::Uniform> viewMatrix;
    std::unique_ptr<juce::OpenGLShaderProgram::Uniform> cameraPosition;
    std::unique_ptr<juce::OpenGLShaderProgram::Uniform> color;
private:
   static juce::OpenGLShaderProgram::Uniform* createUniform (juce::OpenGLShaderProgram& shader, 
                                                             const char* uniformName)
    {
        if (juce::gl::glGetUniformLocation (shader.getProgramID(), uniformName) < 0)
            return nullptr;
        
        return new juce::OpenGLShaderProgram::Uniform (shader, uniformName);
    }
};
class CircularBuffer
{
public:
    CircularBuffer(const int initialAllocation)
    {
        positionsBuffer.resize(initialAllocation); // nearly a second at 48000khz
        writeSampleIndex = 0;
    }
    virtual ~CircularBuffer() {}
    virtual void feed(const juce::AudioBuffer<float>& input)
    {
        jassert(input.getNumChannels() == 3);
        for(int i = 0; i < input.getNumSamples(); i++)
        {
            Position p = {input.getReadPointer(0)[i], input.getReadPointer(1)[i], input.getReadPointer(2)[i] * 0.6f};
            positionsBuffer.set(writeSampleIndex, p); 
            
            writeSampleIndex = (++writeSampleIndex) % positionsBuffer.size();
        }
    }
private:
    struct Position {float x = 0.0f; float y = 0.0f; float z = 0.0f;};
    juce::Array<Position> positionsBuffer;
    int writeSampleIndex;
};
class PointsMesh
{
public:
    PointsMesh (const int numVertices)
    {
        vertexBuffer = std::make_unique<VertexBuffer>(numVertices);
    }
    virtual ~PointsMesh() {}
    virtual void update(void* glVertexWritePtr) = 0;
protected:
    virtual void draw (Attributes& a) // must only be called in GL render loop
    {
        vertexBuffer->bind();
        updateInternal();
        a.enable(); ERROR_CHECK();
        // juce::gl::glPointSize (12.5f); ERROR_CHECK();
        juce::gl::glEnable (juce::gl::GL_VERTEX_PROGRAM_POINT_SIZE);  ERROR_CHECK();// points sized per-vertex
        juce::gl::glDrawArrays (juce::gl::GL_POINTS, (GLint)vertexBuffer->glVertexBuffer, vertexBuffer->numVertices - 2 /* no idea why this is needed*/); ERROR_CHECK();
        a.disable(); ERROR_CHECK();
        juce::gl::glBindBuffer (juce::gl::GL_ARRAY_BUFFER, 0); ERROR_CHECK();
    }
private:
   void updateInternal()
   {
        auto* ptr = juce::gl::glMapBuffer (juce::gl::GL_ARRAY_BUFFER, juce::gl::GL_WRITE_ONLY); ERROR_CHECK();
        if(ptr != nullptr)
        {
            update(ptr);
            juce::gl::glUnmapBuffer (juce::gl::GL_ARRAY_BUFFER); ERROR_CHECK();
        }
   }
    struct VertexBuffer
    {
        VertexBuffer(int initialVertexCount) 
        {
            juce::gl::glGenBuffers (1, &glVertexBuffer);ERROR_CHECK();
            juce::gl::glBindBuffer (juce::gl::GL_ARRAY_BUFFER, glVertexBuffer);ERROR_CHECK();
            numVertices = initialVertexCount;
            color = juce::Colours::indigo;

            juce::gl::glBufferData (juce::gl::GL_ARRAY_BUFFER, initialVertexCount * (int)sizeof(Vertex), 
                                    nullptr, juce::gl::GL_STREAM_DRAW);ERROR_CHECK();
        }
        int size() { return numVertices; }
        
        ~VertexBuffer()
        {
            juce::gl::glDeleteBuffers (1, &glVertexBuffer);
        }
        void bind()
        {
            juce::gl::glBindBuffer (juce::gl::GL_ARRAY_BUFFER, glVertexBuffer);  ERROR_CHECK();
        }
        GLuint glVertexBuffer;
        
        int numVertices;
        juce::Colour color;
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VertexBuffer)
    };
    protected:
    std::unique_ptr<VertexBuffer> vertexBuffer;
};
struct TrajectoryMesh : PointsMesh // must be constructed on GL Initialize
{
    TrajectoryMesh(juce::OpenGLContext& c, tp::StandardVoice* t,  int numVertices = 4096)
      : PointsMesh (numVertices),
        glContext (c),
        voice (t)
        
    {
        if (loadShaders())
        {
            attributes = std::make_unique<Attributes>(*shaders.get());
            uniforms   = std::make_unique<TrajectoryUniforms>(*shaders.get());
        }
        else 
        {
            std::cout << "Trajectory Shaders failed to load\n" << std::endl;
        }
    }
    ~TrajectoryMesh() override {}
    void update (void* glVertexPtr) override 
    {
        std::memcpy (glVertexPtr, voice->getRawData(), sizeof(Vertex) * static_cast<size_t> (vertexBuffer->numVertices));
    }  
    
    void render (const Camera& camera, const juce::Colour color)
    {
        if (!voice->isVoiceActive())
            return; 
            
        juce::gl::glEnable (juce::gl::GL_BLEND);
        juce::gl::glEnable (juce::gl::GL_DEPTH_TEST);
        juce::gl::glDepthMask (juce::gl::GL_FALSE);
        // juce::gl::glBlendFunc (juce::gl::GL_SRC_ALPHA, juce::gl::GL_ONE_MINUS_SRC_ALPHA);
        juce::gl::glBlendFunc (juce::gl::GL_SRC_ALPHA, juce::gl::GL_ONE);
        juce::gl::glEnable (juce::gl::GL_POINT_SPRITE);
        
        if(shaders.get() == nullptr)
            return;
        
        shaders->use();
        if (uniforms->projectionMatrix.get() != nullptr)
            uniforms->projectionMatrix->setMatrix4 (&camera.getProjectionMatrix()[0][0], 1, false);
        if (uniforms->viewMatrix.get() != nullptr)
            uniforms->viewMatrix->setMatrix4 (&camera.getViewMatrix()[0][0], 1, false);
        if (uniforms->color.get() != nullptr)
            uniforms->color->set (color.getRed(), color.getGreen(), color.getBlue());

        PointsMesh::draw (*attributes.get());
        juce::gl::glBindBuffer (juce::gl::GL_ARRAY_BUFFER, 0);
        juce::gl::glDepthMask (juce::gl::GL_TRUE);
    }
private:
    juce::OpenGLContext& glContext;
    tp::StandardVoice* voice; // non-owning 
    std::unique_ptr<juce::OpenGLShaderProgram>  shaders; // tell GL how to draw
    std::unique_ptr<Attributes>           attributes; // tell shaders about vertex data
    std::unique_ptr<TrajectoryUniforms>   uniforms;
    
    bool loadShaders()
    {
        auto vert = juce::String(BinaryData::TrajectoryPoint_vert);
        auto frag = juce::String(BinaryData::TrajectoryPoint_frag);
        shaders = std::make_unique<juce::OpenGLShaderProgram>(glContext);
        bool loaded = false;
        loaded = shaders->addVertexShader(juce::OpenGLHelpers::translateVertexShaderToV3(vert));
        if (!loaded) { std::cout << shaders->getLastError() << std::endl; }
        loaded = shaders->addFragmentShader(juce::OpenGLHelpers::translateFragmentShaderToV3(frag));  
        if (!loaded) { std::cout << shaders->getLastError() << std::endl; }
        loaded = shaders->link();
        if (!loaded) { std::cout << shaders->getLastError() << std::endl; }

        return loaded;
    }
};
struct Trajectories : private tp::WaveTerrainSynthesizerStandard::VoiceListener
{
    Trajectories (juce::OpenGLContext& c, tp::WaveTerrainSynthesizerStandard& wts)
      : context(c)
    {
        wts.setVoiceListener(this);
        voicesReset (wts.getVoices());
    }
    ~Trajectories() override {}
    void render (const Camera& camera, const juce::Colour color)
    {
        for(auto t : trajectories)
            t->render (camera, color);
    }
private:
    void voicesReset (juce::Array<tp::VoiceInterface*> voices) override 
    {
        trajectories.clear();
        for(auto v : voices)
        {
            auto* trajectory = dynamic_cast<tp::StandardVoice*>(v);
            jassert (trajectory != nullptr);

            trajectories.add(std::make_unique<TrajectoryMesh>(context, trajectory));
        }
    }
    juce::OwnedArray<TrajectoryMesh> trajectories;
    juce::OpenGLContext& context;
};