#ifndef BZR_MODELRENDERDATA_H
#define BZR_MODELRENDERDATA_H

#include "Destructable.h"
#include "Noncopyable.h"

class Model;

class ModelRenderData : public Destructable, Noncopyable
{
public:
    ModelRenderData(const Model& model);
    ~ModelRenderData();

    void bind();

    GLsizei indexCount() const;

private:
    void initGeometry(const Model& model);
    void initTexture(const Model& model);

    GLuint _vertexArray;
    GLuint _vertexBuffer;
    GLuint _indexBuffer;
    GLsizei _indexCount;
    GLuint _texture;
};

#endif