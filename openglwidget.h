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
    QOpenGLFunctions_4_2_Core* f;

    QMatrix4x4 m_proj, m_view, m_inv_view;
    GLuint u_proj, u_view, u_trans, u_inv_view, u_grid, u_color, u_shading;

    QOpenGLShaderProgram program, surfacec0_program;
    GLuint surfacec0_u_view, surfacec0_u_proj, surfacec0_u_segments_in, surfacec0_u_segments_out, surfacec0_u_inv_view;

    Camera camera;
    std::unique_ptr<Object> grid;
    std::unique_ptr<Frame> frame;
    std::unique_ptr<Object> bounding_box;

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
    void moveFrame(const float x, const float y);
    void limitFramePosToBounds(QVector3D& pos);

    QVector4D projectFromScreen(const float xpos, const float ypos);

public slots:
    void repaintSlot();
    void restartSimulation();
    void updateSetting();
    void updateState(points_positions_t pos);
};
