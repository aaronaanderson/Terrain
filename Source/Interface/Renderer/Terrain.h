#pragma once

#include <juce_opengl/juce_opengl.h>
#include <BinaryData.h>

#include "glUtility.h"
#include "Camera.h"
#include "Attributes.h"

struct TerrainUniforms
{
    explicit TerrainUniforms (juce::OpenGLShaderProgram& shader)
    { 
        projectionMatrix.reset (createUniform (shader, "projectionMatrix"));
        viewMatrix.reset       (createUniform (shader, "viewMatrix"));
        cameraPosition.reset   (createUniform (shader, "cameraPosition"));
        lightPosition.reset    (createUniform (shader, "lightPosition"));
        terrainIndex.reset     (createUniform (shader, "terrainIndex"));
        modifierA.reset        (createUniform (shader, "a"));
        modifierB.reset        (createUniform (shader, "b"));
        modifierC.reset        (createUniform (shader, "c"));
        modifierD.reset        (createUniform (shader, "d"));
    }
    std::unique_ptr<juce::OpenGLShaderProgram::Uniform> projectionMatrix;
    std::unique_ptr<juce::OpenGLShaderProgram::Uniform> viewMatrix;
    std::unique_ptr<juce::OpenGLShaderProgram::Uniform> cameraPosition;
    std::unique_ptr<juce::OpenGLShaderProgram::Uniform> lightPosition; 
    std::unique_ptr<juce::OpenGLShaderProgram::Uniform> terrainIndex;
    std::unique_ptr<juce::OpenGLShaderProgram::Uniform> modifierA;
    std::unique_ptr<juce::OpenGLShaderProgram::Uniform> modifierB;
    std::unique_ptr<juce::OpenGLShaderProgram::Uniform> modifierC;
    std::unique_ptr<juce::OpenGLShaderProgram::Uniform> modifierD;
private:
   static juce::OpenGLShaderProgram::Uniform* createUniform (juce::OpenGLShaderProgram& shader, 
                                                             const char* uniformName)
    {
        if (juce::gl::glGetUniformLocation (shader.getProgramID(), uniformName) < 0)
            return nullptr;
        return new juce::OpenGLShaderProgram::Uniform (shader, uniformName);
    }
};
struct PlaneMesh
{
    PlaneMesh (const int width, const int height)
    {
        vertexBuffer = std::make_unique<VertexBuffer>(width, height);
    }
    void draw (Attributes& attributes)
    {
        vertexBuffer->bind();
        attributes.enable();
        juce::gl::glDrawElements (juce::gl::GL_TRIANGLES, vertexBuffer->numIndices, juce::gl::GL_UNSIGNED_INT, nullptr);ERROR_CHECK();
        attributes.disable();
    }
private:
    struct VertexBuffer
    {
        VertexBuffer(const int w, const int h) 
        {
            juce::gl::glGenBuffers (1, &glVertexBuffer); ERROR_CHECK();
            juce::gl::glBindBuffer (juce::gl::GL_ARRAY_BUFFER, glVertexBuffer); ERROR_CHECK();
            numVertices = w * h;
            // https://stackoverflow.com/questions/5915753/generate-a-plane-with-triangle-strips
            juce::Array<float> initialVertexData;
            initialVertexData.resize(w * h * 3);
            for(int x = 0; x < w; x++)
            {
                float xNorm = juce::jmap<float>((float)x, 0, (float)w, -1.0f, 1.0f);
                for(int y = 0; y < h; y++)
                {
                    float yNorm = juce::jmap<float>((float)y, 0, (float)h, -1.0f, 1.0f);
                    int index = x * w + y;
                    initialVertexData.set(3 * index + 0, xNorm); // x
                    initialVertexData.set(3 * index + 1, yNorm); // y
                    initialVertexData.set(3 * index + 2, 0.0f);  // Z
                }
            }
            juce::gl::glBufferData (juce::gl::GL_ARRAY_BUFFER, numVertices * (int)sizeof(Vertex), 
                                    initialVertexData.getRawDataPointer(), juce::gl::GL_DYNAMIC_DRAW); ERROR_CHECK();
            
            juce::Array<int> initialIndexData; 
            for (int x = 0; x < w - 1; x++)
            {
                for (int y = 0; y < h - 1; y++)
                {
                    int bl = (x * h) + y;
                    int br = ((x + 1) * h) + y;
                    int tl = (x * h) + (y + 1);
                    int tr = ((x + 1) * h) + (y + 1);
                    
                    initialIndexData.add(bl);
                    initialIndexData.add(tl);
                    initialIndexData.add(br);
                    initialIndexData.add(br);
                    initialIndexData.add(tl);
                    initialIndexData.add(tr);
                }
            }
            numIndices = initialIndexData.size();
            juce::gl::glGenBuffers (1, &glIndexBuffer); ERROR_CHECK();
            juce::gl::glBindBuffer (juce::gl::GL_ELEMENT_ARRAY_BUFFER, glIndexBuffer); ERROR_CHECK();
            juce::gl::glBufferData (juce::gl::GL_ELEMENT_ARRAY_BUFFER, numIndices * (int) sizeof (juce::uint32),
                                                   initialIndexData.getRawDataPointer(), juce::gl::GL_DYNAMIC_DRAW);
        }
        ~VertexBuffer()
        {
            juce::gl::glDeleteBuffers (1, &glVertexBuffer); ERROR_CHECK();
            juce::gl::glDeleteBuffers (1, &glIndexBuffer);  ERROR_CHECK();
        }
        void bind()
        {
            juce::gl::glBindBuffer (juce::gl::GL_ARRAY_BUFFER, glVertexBuffer); ERROR_CHECK();
            juce::gl::glBindBuffer (juce::gl::GL_ELEMENT_ARRAY_BUFFER, glIndexBuffer); ERROR_CHECK();
        }
        GLuint glVertexBuffer;
        GLuint glIndexBuffer;
        
        int numVertices;
        int numIndices;
        juce::Colour color;
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VertexBuffer)
    };
    std::unique_ptr<VertexBuffer> vertexBuffer;    
};
class Terrain
{
public:
    Terrain (juce::OpenGLContext& c)
      : glContext (c), 
        mesh (512, 512)
    {
        if (loadShaders())
        {
            uniforms = std::make_unique<TerrainUniforms> (*shaders.get());
            attributes = std::make_unique<Attributes> (*shaders.get());
        }
    }
    void render (const Camera& camera, int index, float modA, float modB, float modC, float modD)
    {
        juce::gl::glDisable (juce::gl::GL_BLEND);
        juce::gl::glEnable (juce::gl::GL_DEPTH_TEST);
        juce::gl::glPolygonMode (juce::gl::GL_FRONT_AND_BACK, juce::gl::GL_FILL);
        if(shaders.get() == nullptr)
            return;
            
        shaders->use();

        if (uniforms->projectionMatrix.get() != nullptr)
            uniforms->projectionMatrix->setMatrix4 (&camera.getProjectionMatrix()[0][0], 1, false);
        if (uniforms->viewMatrix.get() != nullptr)
            uniforms->viewMatrix->setMatrix4 (&camera.getViewMatrix()[0][0], 1, false);
        if (uniforms->lightPosition.get() != nullptr)
        {
            phase = phase + 0.005f;
            uniforms->lightPosition->set (std::sin (phase) * 8, std::cos (phase) * 8, 2.0f);
        }
        if (uniforms->terrainIndex.get() != nullptr) uniforms->terrainIndex->set (index);
        if (uniforms->modifierA.get() != nullptr) uniforms->modifierA->set (modA);
        if (uniforms->modifierB.get() != nullptr) uniforms->modifierB->set (modB);
        if (uniforms->modifierC.get() != nullptr) uniforms->modifierC->set (modC);
        if (uniforms->modifierD.get() != nullptr) uniforms->modifierD->set (modD);

        mesh.draw (*attributes.get());
        // juce::gl::glBindBuffer (juce::gl::GL_ARRAY_BUFFER, 0);
        // juce::gl::glPolygonMode (juce::gl::GL_FRONT_AND_BACK, juce::gl::GL_FILL );
    }
private:
    juce::OpenGLContext& glContext;
    std::unique_ptr<juce::OpenGLShaderProgram> shaders;
    std::unique_ptr<TerrainUniforms>           uniforms;
    std::unique_ptr<Attributes>                attributes;
    PlaneMesh                                  mesh;
    float phase = 0.0f;

    bool loadShaders()
    {
        auto vert = juce::String(BinaryData::Terrain_vert);
        auto frag = juce::String(BinaryData::Terrain_frag);
        shaders = std::make_unique<juce::OpenGLShaderProgram>(glContext);
        bool loaded = false;
        loaded = shaders->addVertexShader(juce::OpenGLHelpers::translateVertexShaderToV3(vert));
        if(!loaded) { std::cout << shaders->getLastError() << std::endl; }
        loaded = shaders->addFragmentShader(juce::OpenGLHelpers::translateFragmentShaderToV3(frag));  
        if(!loaded) { std::cout << shaders->getLastError() << std::endl; }
        loaded = shaders->link();
        if(!loaded) { std::cout << shaders->getLastError() << std::endl; }

        return loaded;        
    }
};