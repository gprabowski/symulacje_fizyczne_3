#include "openglwidget.h"
#include <QDebug>
#include <fstream>
#include <iostream>
#include <memory>
#include <queue>

#include <QFileDialog>
#include <QMouseEvent>
#include <QOpenGLShaderProgram>
#include <QOpenGLVersionFunctionsFactory>

OpenGLWidget::OpenGLWidget(QWidget* parent)
    : QOpenGLWidget(parent)
{
    setFocusPolicy(Qt::StrongFocus);
    m_proj = Identity();
    camera.translation = QVector3D(0, 0, -10.0f);

    camera.RotateX(-0.8f);
    camera.RotateY(0.8f);

    m_view = camera.Matrix();

    setMouseTracking(true);
}

OpenGLWidget::~OpenGLWidget()
{
    makeCurrent();
}

void OpenGLWidget::updateCamera()
{
    makeCurrent();
    m_view = camera.Matrix();
    m_inv_view = m_view.inverted();
    repaint();
}

void OpenGLWidget::moveFrame(const float x, const float y)
{
    const QVector3D direction_from_screen = projectFromScreen(x, y).toVector3D();
    const QVector4D camera_pos = Translation(camera.translation) * QVector4D(0, 0, 0, 1) * camera.Rotation() - camera.center.toVector4D();
    const QVector3D frame_pos = (camera_pos.toVector3D() + direction_from_screen);
    frame->translateTo(frame_pos);
    simulation_thread->changeFramePosition(frame_pos);
}

void OpenGLWidget::initializeGL()
{
    simulation_thread = std::make_unique<SimulationThread>(simulation_settings);
    connect(simulation_thread.get(), &SimulationThread::pointsPositionChanged, this, &OpenGLWidget::updateState);

    initializeOpenGLFunctions();
    QOpenGLVersionProfile version;
    version.setProfile(QSurfaceFormat::CoreProfile);
    version.setVersion(4, 2);
    f = reinterpret_cast<QOpenGLFunctions_4_2_Core*>(QOpenGLVersionFunctionsFactory::get(version, QOpenGLContext::currentContext()));

    grid = std::unique_ptr<Object>(new Object({ { 1.0f, 0.0f, 1.0f },
                                                  { 1.0f, 0.0f, -1.0f },
                                                  { -1.0f, 0.0f, -1.0f },
                                                  { -1.0f, 0.0f, 1.0f } },
        { 0, 1, 3, 1, 2, 3 }, f));
    grid->scale(QVector3D(300.0f, 1.0f, 300.0f), QVector3D(0, 0, 0));
    grid->mode = DrawMode::Triangles;
    grid->translation.setY(0.001f);

    cube = std::make_unique<Cube>(f);
    cube->rotation = base_cube_rotation;

    points = std::make_unique<Points>(1000, f);

    points_positions_t pnts;

    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            for (int k = 0; k < 4; k++)
            {
                pnts[i][j][k] = QVector3D(i - 1.5f, j - 1.5f, k - 1.5f) + QVector3D(float(rand() % 51) / 100.0f - 0.25f, float(rand() % 51) / 100.0f - 0.25f, float(rand() % 51) / 100.0f - 0.25f);
            }
        }
    }

    //bezierCube = std::make_unique<BezierCube>(pnts, f);

    frame = std::make_unique<Frame>(QVector3D(0, 0, 0), 3, pnts, f);

    program.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/vshader.glsl");
    program.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/fshader.glsl");
    program.link();
    program.bind();
    u_view = program.uniformLocation("m_view");
    u_proj = program.uniformLocation("m_proj");
    u_trans = program.uniformLocation("m_trans");
    u_inv_view = program.uniformLocation("m_inv_view");
    u_color = program.uniformLocation("m_color");
    u_grid = program.uniformLocation("grid");
    u_shading = program.uniformLocation("shading");
    program.setUniformValue(u_view, m_view);
    program.setUniformValue(u_proj, m_proj);
    program.setUniformValue(u_trans, Identity());

    surfacec0_program.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/surface_v.glsl");
    surfacec0_program.addShaderFromSourceFile(QOpenGLShader::TessellationControl, ":/surface_tcs.glsl");
    surfacec0_program.addShaderFromSourceFile(QOpenGLShader::TessellationEvaluation, ":/surfacec0_tes.glsl");
    surfacec0_program.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/surface_f.glsl");
    surfacec0_program.link();
    surfacec0_program.bind();

    surfacec0_u_view = surfacec0_program.uniformLocation("m_view");
    surfacec0_u_proj = surfacec0_program.uniformLocation("m_proj");
    surfacec0_u_segments_in = surfacec0_program.uniformLocation("segments_in");
    surfacec0_u_segments_out = surfacec0_program.uniformLocation("segments_out");
    surfacec0_u_inv_view = surfacec0_program.uniformLocation("m_inv_view");
    surfacec0_program.setUniformValue(surfacec0_u_view, m_view);
    surfacec0_program.setUniformValue(surfacec0_u_inv_view, m_inv_view);
    surfacec0_program.setUniformValue(surfacec0_u_proj, m_proj);
    surfacec0_program.setUniformValue(surfacec0_u_segments_in, 20);
    surfacec0_program.setUniformValue(surfacec0_u_segments_out, 20);

    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_LINE_SMOOTH);

    connect(this, &OpenGLWidget::frameSwapped, this, QOverload<>::of(&OpenGLWidget::update));
}

void OpenGLWidget::resizeGL(int w, int h)
{
    // m_proj = OrthographicProjection(-float(w) / 100.0f, float(w) / 100.0f, -float(h) / 100.0f, float(h) / 100.0f);
    m_proj = PerspectiveProjection(float(h) / float(w), M_PI / 2.0f, 50.0f, 0.01f);

    program.bind();
    program.setUniformValue(u_proj, m_proj);
}

void OpenGLWidget::paintGL()
{
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glClearColor(0.19f, 0.21f, 0.23f, 1.0f);
    // glClearColor(0.0, 0.0, 0.0, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    program.bind();
    program.setUniformValue(u_view, m_view);
    program.setUniformValue(u_inv_view, m_inv_view);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glDisable(GL_DEPTH_TEST);

    program.setUniformValue(u_grid, true);
    program.setUniformValue(u_trans, grid->Matrix());
    program.setUniformValue(u_shading, false);
    grid->Render();
    program.setUniformValue(u_grid, false);

    surfacec0_program.bind();
    surfacec0_program.setUniformValue(surfacec0_u_view, m_view);
    surfacec0_program.setUniformValue(surfacec0_u_inv_view, m_inv_view);
    surfacec0_program.setUniformValue(surfacec0_u_proj, m_proj);
    if (simulation_settings.show_jelly)
        frame->bezier_cube->Render();

    glEnable(GL_DEPTH_TEST);

    program.bind();
    program.setUniformValue(u_shading, false);
    program.setUniformValue(u_view, m_view);
    program.setUniformValue(u_inv_view, m_inv_view);
    program.setUniformValue(u_trans, frame->Matrix());
    program.setUniformValue(u_color, QVector4D(0.0f, 1.0f, 0.0f, 1.0f));
    if (simulation_settings.show_control_frame)
        frame->Render();

    if (simulation_settings.show_control_points)
    {
        program.setUniformValue(u_trans, Identity());
        program.setUniformValue(u_color, QVector4D(1.0f, 0.0f, 1.0f, 1.0f));
        frame->bezier_cube->RenderPoints();
        frame->bezier_cube->net->Render();
    }
}

void OpenGLWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (draggedRight && event->buttons().testFlag(Qt::RightButton))
    {
        const QPointF diff = lastMousePoint - event->pos();
        if (diff.manhattanLength() < 10)
        {
            return;
        }
        lastMousePoint = event->pos();
        const float s = std::max(width(), height());
        if (event->modifiers().testFlag(Qt::ShiftModifier))
        {
            QVector3D tr_y = camera.inversedRotation.map(RotationX(M_PI / 2.0f).map(QVector3D(0, 0, diff.y() / 100.0f)));
            QVector3D tr_x = camera.inversedRotation.map(RotationY(M_PI / 2.0f).map(QVector3D(0, 0, diff.x() / 100.0f)));
            camera.center += tr_x + tr_y;
        }
        else
        {
            const float y_angle = 2 * M_PI * diff.x() / s;
            const float x_angle = 2 * M_PI * diff.y() / s;
            camera.RotateX(x_angle);
            camera.RotateY(y_angle);
        }
        updateCamera();
    }
    else if (event->buttons().testFlag(Qt::LeftButton))
    {
        const float x = float(2.0f * event->pos().x()) / float(width()) - 1.0f;
        const float y = 1.0f - float(2.0f * event->pos().y()) / float(height());
        moveFrame(x, y);
    }
}

void OpenGLWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::RightButton)
    {
        draggedRight = true;
        lastMousePoint = event->pos();
    }
    if (event->button() == Qt::LeftButton)
    {
        const float x = float(2.0f * event->pos().x()) / float(width()) - 1.0f;
        const float y = 1.0f - float(2.0f * event->pos().y()) / float(height());
        moveFrame(x, y);
    }
}

void OpenGLWidget::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::RightButton)
    {
        draggedRight = false;
        lastMousePoint = QPoint(-1, -1);
    }
    if (event->button() == Qt::LeftButton)
    {
        draggedLeft = false;
        lastMousePoint = QPoint(-1, -1);
    }
}

void OpenGLWidget::keyPressEvent(QKeyEvent* event)
{
    switch (event->key())
    {

    case Qt::Key::Key_Up:
    {
        camera.translation.setZ(camera.translation.z() + 0.3f);
        updateCamera();
        break;
    }
    case Qt::Key::Key_Down:
    {
        camera.translation.setZ(camera.translation.z() - 0.3f);
        updateCamera();
        break;
    }
    }
}

void OpenGLWidget::wheelEvent(QWheelEvent* event)
{
    float d = camera.translation.z();
    d += event->pixelDelta().y() / 100.0f;
    if (abs(d) < 0.1f)
        d = -0.1f;
    camera.translation.setZ(d);
    updateCamera();
}

void OpenGLWidget::repaintSlot()
{
    repaint();
}

void OpenGLWidget::restartSimulation()
{
    simulation_thread->restart(simulation_settings);
    // throw std::logic_error("not implemented");
}

void OpenGLWidget::updateSetting()
{
    simulation_thread->updateSettings(simulation_settings);
    throw std::logic_error("not implemented");
}

void OpenGLWidget::updateState(points_positions_t pos)
{
    frame->updatePoints(pos);
}

void OpenGLWidget::resetPoints(const int max_points)
{
    makeCurrent();
    points.reset(new Points(max_points, f));
}

QVector4D OpenGLWidget::projectFromScreen(const float xpos, const float ypos)
{
    QVector4D screen_pos = QVector4D(xpos, ypos, 1, 1);
    QVector4D camera_pos = Translation(camera.translation) * QVector4D(0, 0, 0, 1) * camera.Rotation();
    camera_pos -= camera.center.toVector4D();
    QMatrix4x4 inv_proj = m_proj.inverted();
    QMatrix4x4 inv_view = m_view.inverted();
    QVector4D v = inv_proj * screen_pos;
    v.setW(0);
    v.setZ(1);
    v = inv_view * v;
    v.setW(0);
    v.normalize();
    v *= -camera.translation.z();
    return v;
}
