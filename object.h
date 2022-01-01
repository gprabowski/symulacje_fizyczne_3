#pragma once
#include <array>
#include <vector>

#include <QMatrix4x4>
#include <QOpenGLFunctions_4_2_Core>
#include <QOpenGLTexture>
#include <QQuaternion>
#include <QVector3D>

#include "functions.h"

struct TriangleIndices
{
    unsigned int indices[3];
};

struct EdgeIndices
{
    unsigned int indices[2];
};

struct IndicesBuffer : public std::vector<unsigned int>
{
    void add_edge(const EdgeIndices& e)
    {
        emplace_back(e.indices[0]);
        emplace_back(e.indices[1]);
    }
    void add_triangle(const TriangleIndices& t)
    {
        emplace_back(t.indices[0]);
        emplace_back(t.indices[1]);
        emplace_back(t.indices[2]);
    }

    IndicesBuffer(std::initializer_list<unsigned int> init)
        : std::vector<unsigned int>(init)
    {
    }
    IndicesBuffer()
        : std::vector<unsigned int>()
    {
    }
};

enum class DrawMode
{
    Edges,
    Triangles,
    Points
};

class Transformation
{
public:
    QVector3D translation = QVector3D(0.0f, 0.0f, 0.0f);
    QQuaternion rotation;
    QVector3D scaleVector = QVector3D(1.0f, 1.0f, 1.0f);

    QMatrix4x4 Matrix() const;
    QMatrix4x4 Rotation() const;
};

class Object : public Transformation
{
public:
    Object(const std::vector<QVector3D>& p, const IndicesBuffer& i, QOpenGLFunctions_4_2_Core* f);
    Object(QOpenGLFunctions_4_2_Core* f);
    ~Object();

    virtual void Render();

    virtual void translate(QVector3D v);
    virtual void scale(QVector3D s, QVector3D p);

    std::string name = "";
    bool selected = false;

    std::vector<QVector3D> vertices;
    IndicesBuffer indices;
    void ModifyOpenglData();

    DrawMode mode = DrawMode::Edges;

    QOpenGLFunctions_4_2_Core* f;
    unsigned int VAO, VBO, EBO;

protected:
    bool modified = false;
    Object() {};
};

class Camera : public Transformation
{
public:
    QVector3D center = QVector3D(0, 0, 0);
    float rotationY = 0;
    float rotationX = 0;
    QMatrix4x4 inversedRotation = Identity();
    Camera() {};
    ~Camera() {};
    void RotateX(float angle);
    void RotateY(float angle);
    QMatrix4x4 Matrix() const;
    QMatrix4x4 Rotation() const;
};

class BezierCube : public Object
{
public:
    std::unique_ptr<Object> net;

    virtual void Render() override;
    void RenderPoints();
    void updatePoints(const std::array<std::array<std::array<QVector3D, 4>, 4>, 4>& p);

    BezierCube(const std::array<std::array<std::array<QVector3D, 4>, 4>, 4>& p, QOpenGLFunctions_4_2_Core* f);
};

class Frame : public Object
{
public:
    std::unique_ptr<BezierCube> bezier_cube;

    Frame(const QVector3D pos, const float a, const std::array<std::array<std::array<QVector3D, 4>, 4>, 4>& p, QOpenGLFunctions_4_2_Core* f);
    void updatePoints(const std::array<std::array<std::array<QVector3D, 4>, 4>, 4>& p);
};

class Cube : public Object
{
private:
    Cube(const std::vector<QVector3D>& p, const IndicesBuffer& i, QOpenGLFunctions_4_2_Core* f);

public:
    std::unique_ptr<Object> diagonal;
    Cube(QOpenGLFunctions_4_2_Core* f);
};

class Points : public Object
{
    int current = 0;
    int last = 0;
    int max_vertices = 0;

public:
    Points(const int n, QOpenGLFunctions_4_2_Core* f);
    virtual void Render() override;
    void AddPoint(const QVector3D& p);
    void ResetPoints();
};
