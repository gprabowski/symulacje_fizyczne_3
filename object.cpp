#include "object.h"

#include <algorithm>
#include <numeric>

#include <QVector2D>

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

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

Object::Object(QOpenGLFunctions_4_2_Core* f, std::string filename)
    : Object(f)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename.c_str()))
    {
        throw std::runtime_error(warn + err);
    }
    const auto& ind = shapes.front().mesh.indices;
    const auto& points = attrib.vertices;
    vertices.reserve(points.size() / 3);
    for (int i = 0; i < points.size() / 3; i++)
        vertices.push_back(QVector3D(points[3 * i], points[3 * i + 1], points[3 * i + 2]));
    indices.reserve(ind.size());
    for (int i = 0; i < ind.size(); i++)
        indices.push_back(ind[i].vertex_index);

    float min = vertices.front().x(), max = vertices.front().x();

    for (const auto& v : vertices)
    {
        min = std::min(min, v.x());
        min = std::min(min, v.y());
        min = std::min(min, v.z());

        max = std::max(max, v.x());
        max = std::max(max, v.y());
        max = std::max(max, v.z());
    }
    for (auto& v : vertices)
    {
        v -= QVector3D(min, min, min);
        v /= QVector3D(max - min, max - min, max - min);
    }

    mode = DrawMode::Triangles;
    ModifyOpenglData();
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

void BezierCube::Render()
{
    f->glBindVertexArray(VAO);
    f->glPatchParameteri(GL_PATCH_VERTICES, 16);
    f->glDrawElements(GL_PATCHES, indices.size(), GL_UNSIGNED_INT, 0);
    f->glBindVertexArray(0);
}

void BezierCube::RenderPoints()
{
    f->glBindVertexArray(VAO);
    f->glDrawElements(GL_POINTS, indices.size(), GL_UNSIGNED_INT, 0);
    f->glBindVertexArray(0);
}

void BezierCube::updatePoints(const std::array<std::array<std::array<QVector3D, 4>, 4>, 4>& p)
{
    const auto toIdx = [&](const int i, const int j, const int k) -> unsigned int
    { return i * 16 + j * 4 + k; };

    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            for (int k = 0; k < 4; k++)
            {
                vertices[toIdx(i, j, k)] = p[i][j][k];
                net->vertices[toIdx(i, j, k)] = p[i][j][k];
            }
        }
    }
    ModifyOpenglData();
    net->ModifyOpenglData();
}

BezierCube::BezierCube(const std::array<std::array<std::array<QVector3D, 4>, 4>, 4>& p, QOpenGLFunctions_4_2_Core* f)
    : Object(f)
{

    indices.reserve(16 * 6);
    const auto toIdx = [&](const int i, const int j, const int k) -> unsigned int
    { return i * 16 + j * 4 + k; };

    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            indices.push_back(toIdx(0, i, j));
        }
    }
    for (int i = 3; i >= 0; i--)
    {
        for (int j = 0; j < 4; j++)
        {
            indices.push_back(toIdx(3, i, j));
        }
    }

    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            indices.push_back(toIdx(i, 0, j));
        }
    }
    for (int i = 3; i >= 0; i--)
    {
        for (int j = 0; j < 4; j++)
        {
            indices.push_back(toIdx(i, 3, j));
        }
    }

    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            indices.push_back(toIdx(i, j, 0));
        }
    }
    for (int i = 3; i >= 0; i--)
    {
        for (int j = 0; j < 4; j++)
        {
            indices.push_back(toIdx(i, j, 3));
        }
    }

    net = std::make_unique<Object>(f);
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            for (int k = 0; k < 3; k++)
            {
                net->indices.add_edge({ toIdx(i, j, k), toIdx(i + 1, j, k) });
                net->indices.add_edge({ toIdx(i, j, k), toIdx(i, j + 1, k) });
                net->indices.add_edge({ toIdx(i, j, k), toIdx(i, j, k + 1) });
            }
        }
    }
    for (int j = 0; j < 3; j++)
    {
        for (int k = 0; k < 3; k++)
        {
            net->indices.add_edge({ toIdx(3, j, k), toIdx(3, j + 1, k) });
            net->indices.add_edge({ toIdx(3, j, k), toIdx(3, j, k + 1) });
            net->indices.add_edge({ toIdx(j, 3, k), toIdx(j + 1, 3, k) });
            net->indices.add_edge({ toIdx(j, 3, k), toIdx(j, 3, k + 1) });
            net->indices.add_edge({ toIdx(j, k, 3), toIdx(j + 1, k, 3) });
            net->indices.add_edge({ toIdx(j, k, 3), toIdx(j, k + 1, 3) });
        }
    }
    for (int k = 0; k < 3; k++)
    {
        net->indices.add_edge({ toIdx(3, 3, k), toIdx(3, 3, k + 1) });
        net->indices.add_edge({ toIdx(3, k, 3), toIdx(3, k + 1, 3) });
        net->indices.add_edge({ toIdx(k, 3, 3), toIdx(k + 1, 3, 3) });
    }

    vertices.resize(64);
    net->vertices.resize(64);
    updatePoints(p);
}

Frame::Frame(const QVector3D pos, const float a, const std::array<std::array<std::array<QVector3D, 4>, 4>, 4>& p, QOpenGLFunctions_4_2_Core* f)
    : Object({ //cube
                 { -0.5f * a, 0.5f * a, 0.5f * a },
                 { -0.5f * a, -0.5f * a, 0.5f * a },
                 { 0.5f * a, 0.5f * a, 0.5f * a },
                 { 0.5f * a, -0.5f * a, 0.5f * a },
                 { -0.5f * a, 0.5f * a, -0.5f * a },
                 { -0.5f * a, -0.5f * a, -0.5f * a },
                 { 0.5f * a, 0.5f * a, -0.5f * a },
                 { 0.5f * a, -0.5f * a, -0.5f * a },
                 //8 bezier cube verticles
                 p[0][3][3] - pos, p[0][0][3] - pos, p[3][3][3] - pos, p[3][0][3] - pos,
                 p[0][3][0] - pos, p[0][0][0] - pos, p[3][3][0] - pos, p[3][0][0] - pos },
        { /*cube*/ 0, 1, 1, 3, 3, 2, 2, 0, 4, 5, 5, 7, 7, 6, 6, 4, 0, 4, 1, 5, 3, 7, 2, 6,
            /*springs*/ 0, 8, 1, 9, 2, 10, 3, 11, 4, 12, 5, 13, 6, 14, 7, 15 },
        f)
{
    translation = pos;
    mode = DrawMode::Edges;
    bezier_cube = std::make_unique<BezierCube>(p, f);
}

void Frame::updatePoints(const std::array<std::array<std::array<QVector3D, 4>, 4>, 4>& p)
{
    bezier_cube->updatePoints(p);

    vertices[8] = p[0][3][3] - translation;
    vertices[9] = p[0][0][3] - translation;
    vertices[10] = p[3][3][3] - translation;
    vertices[11] = p[3][0][3] - translation;
    vertices[12] = p[0][3][0] - translation;
    vertices[13] = p[0][0][0] - translation;
    vertices[14] = p[3][3][0] - translation;
    vertices[15] = p[3][0][0] - translation;
    ModifyOpenglData();
}

void Frame::translateTo(QVector3D pos)
{
    for (int i = 8; i < 16; i++)
        vertices[i] -= (pos - translation);
    ModifyOpenglData();
    translation = pos;
}
