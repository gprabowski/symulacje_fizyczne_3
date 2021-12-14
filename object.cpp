#include "object.h"

#include <algorithm>
#include <numeric>

#include <QVector2D>

QMatrix4x4 Transformation::Matrix() const
{
    return Translation(translation) * Rotation() * Scale(scaleVector);
}

QMatrix4x4 Transformation::Rotation() const
{

    return M3to4(rotation.toRotationMatrix());
}

void Object::ModifyOpenglData()
{
    f->glBindVertexArray(VAO);

    f->glBindBuffer(GL_ARRAY_BUFFER, VBO);
    f->glBufferData(GL_ARRAY_BUFFER, sizeof(QVector3D) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

    f->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    f->glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices.size(), indices.data(), GL_STATIC_DRAW);

    f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    f->glEnableVertexAttribArray(0);

    f->glBindBuffer(GL_ARRAY_BUFFER, 0);
    f->glBindVertexArray(0);
}

Object::Object(const std::vector<QVector3D>& p, const IndicesBuffer& i, QOpenGLFunctions_4_2_Core* f)
    : vertices(p)
    , indices(i)
    , f(f)
{

    f->glGenVertexArrays(1, &VAO);
    f->glGenBuffers(1, &VBO);
    f->glGenBuffers(1, &EBO);

    ModifyOpenglData();
}

Object::Object(QOpenGLFunctions_4_2_Core* f)
    : f(f)
{
    f->glGenVertexArrays(1, &VAO);
    f->glGenBuffers(1, &VBO);
    f->glGenBuffers(1, &EBO);
}

Object::~Object()
{
    f->glDisableVertexAttribArray(0);
    f->glDeleteVertexArrays(1, &VAO);
    f->glDeleteBuffers(1, &VBO);
    f->glDeleteBuffers(1, &EBO);
}

void Object::Render()
{
    if (modified)
    {
        modified = false;
        ModifyOpenglData();
    }

    f->glBindVertexArray(VAO);
    switch (mode)
    {
    case DrawMode::Edges:
        f->glDrawElements(GL_LINES, indices.size(), GL_UNSIGNED_INT, 0);
        break;
    case DrawMode::Triangles:
        f->glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        break;
    case DrawMode::Points:
        f->glDrawElements(GL_POINTS, indices.size(), GL_UNSIGNED_INT, 0);
        break;
    }
    f->glBindVertexArray(0);
}

void Object::translate(QVector3D v)
{
    translation += v;
}

void Object::scale(QVector3D s, QVector3D p)
{
    scaleVector += s;
    translation += (translation - p) * s;
}

void Camera::RotateX(float angle)
{
    rotationX += angle;
    inversedRotation = Rotation().inverted();
}

void Camera::RotateY(float angle)
{
    rotationY += angle;
    inversedRotation = Rotation().inverted();
}

QMatrix4x4 Camera::Matrix() const
{
    return Translation(-translation) * RotationX(rotationX) * RotationY(rotationY) * Translation(center);
}

QMatrix4x4 Camera::Rotation() const
{
    return RotationX(rotationX) * RotationY(rotationY);
}

Cube::Cube(QOpenGLFunctions_4_2_Core* f)
    : Object({ { 0.0f, 1.0f, 1.0f },
                 { 0.0f, 0.0f, 1.0f },
                 { 1.0f, 1.0f, 1.0f },
                 { 1.0f, 0.0f, 1.0f },
                 { 0.0f, 1.0f, 0.0f },
                 { 0.0f, 0.0f, 0.0f },
                 { 1.0f, 1.0f, 0.0f },
                 { 1.0f, 0.0f, 0.0f } },
        { 0, 2, 3, 0, 3, 1,
            2, 6, 7, 2, 7, 3,
            6, 4, 5, 6, 5, 7,
            4, 0, 1, 4, 1, 5,
            0, 4, 6, 0, 6, 2,
            1, 5, 7, 1, 7, 3 },
        f)
{
    mode = DrawMode::Triangles;
    diagonal = std::make_unique<Object>(std::vector<QVector3D>({ { 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f } }), IndicesBuffer({ 0, 1 }), f);
}

Points::Points(const int n, QOpenGLFunctions_4_2_Core* f)
    : Object(f)
{
    max_vertices = n;
    vertices.resize(n);
    indices.resize(n);
    for (int i = 0; i < n; i++)
        indices[i] = i;
    ModifyOpenglData();
}

void Points::Render()
{
    ModifyOpenglData();
    f->glBindVertexArray(VAO);
    f->glDrawElements(GL_POINTS, last, GL_UNSIGNED_INT, 0);
    f->glBindVertexArray(0);
}

void Points::AddPoint(const QVector3D& p)
{
    vertices[current++] = p;
    last++;
    last = std::min(last, max_vertices);
    current %= max_vertices;
}

void Points::ResetPoints()
{
    current = 0;
    last = 0;
}
