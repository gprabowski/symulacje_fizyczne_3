#pragma once
#include <set>
#include <vector>

#include <QMatrix4x4>
#include <QOpenGLFunctions_4_2_Core>
#include <QOpenGLShaderProgram>
#include <QOpenGLWidget>

#include "object.h"
#include "simulationthread.h"

class OpenGLWidget : public QOpenGLWidget,
                     public QOpenGLFunctions_4_2_Core
{
    Q_OBJECT

public:
    explicit OpenGLWidget(QWidget* parent = nullptr);
    ~OpenGLWidget();

    void paintGL() override;
    bool display_cube = true;
    bool display_diagonal = true;
    bool display_path = true;
    SimulationSettings simulation_settings {};
    std::unique_ptr<SimulationThread> simulation_thread;

private:
    std::unique_ptr<Cube> cube;
    std::unique_ptr<Points> points;
    const QQuaternion base_cube_rotation = QQuaternion::rotationTo(QVector3D(1.0f, 1.0f, 1.0f), QVector3D(0, 1.0f, 0));

    QOpenGLFunctions_4_2_Core* f;

    QMatrix4x4 m_proj, m_view, m_inv_view;
    GLuint u_proj, u_view, u_trans, u_inv_view, u_grid, u_color, u_gray;

    Camera camera;

    std::unique_ptr<Object> grid;

    QOpenGLShaderProgram program;

    void initializeGL() override;
    void resizeGL(int w, int h) override;

    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

    QPoint lastMousePoint = QPoint(-1, -1);
    bool draggedRight = false;
    bool draggedLeft = false;

    void setCamera();
    void updateCamera();
    void frame_position_changed(const frame_position_t& frame);

public slots:
    void repaintSlot();
    void restartSimulation();
    void updateSetting();
    // to wygląda źle ale przez Return Value Optimization jeśli dobrze
    // zrobione to nie będzie takie złe myślę
    void updateState(point_positions_t pos);
    void resetPoints(const int max_points);
};
