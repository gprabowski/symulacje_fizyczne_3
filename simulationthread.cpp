#include "simulationthread.h"
#include <QDebug>
#include <QtMath>
#include <cmath>
#include <type_traits>

void SimulationThread::precalculate()
{
    // Precalculate I with respect to the fixed vertex
    throw std::logic_error("not implemented");
}

SimulationThread::SimulationThread(const SimulationSettings& s)
{
    settings = s;
    s_timer.setTimerType(Qt::PreciseTimer);
    connect(&s_timer, &QTimer::timeout, this, &SimulationThread::update);
    s_timer.moveToThread(this);
}

inline QVector3D mult(const QMatrix3x3& m, const QVector3D v)
{
    return QVector3D {
        v[0] * m(0, 0) + v[1] * m(0, 1) + v[2] * m(0, 2),
        v[0] * m(1, 0) + v[1] * m(1, 1) + v[2] * m(1, 2),
        v[0] * m(2, 0) + v[1] * m(2, 1) + v[2] * m(2, 2)
    };
}

void SimulationThread::update() noexcept
{
    point_positions_t p;
    // to wygląda źle ale przez Return Value Optimization jeśli dobrze
    // zrobione to nie będzie takie złe myślę
    emit positionChanged(p);
    throw std::logic_error("not implemented");
}

void SimulationThread::updateSettings(const SimulationSettings& s)
{
    settings = s;
    precalculate();
}

void SimulationThread::restart(const SimulationSettings& s)
{
    exit();
    wait();
    settings = s;
    precalculate();
    throw std::logic_error("not implemented");
    start();
}

void SimulationThread::run()
{
    s_timer.start(settings.dt_ms);
    exec();
}

// też RVO powinno ratować
void SimulationThread::frame_changed(frame_position_t frame)
{
    throw std::logic_error("not implemented");
}
