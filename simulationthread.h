#ifndef SPRINGTHREAD_H
#define SPRINGTHREAD_H

#include <QQuaternion>
#include <QThread>
#include <QTimer>

struct SimulationSettings
{
    float edge_length;
    float density;
    float deviation;
    float angular_velocity;
    float dt_ms;
    bool gravity;
};

// TODO: switch to direct modification of class fields
struct RK4_ret
{
    QVector3D ang_vel_t;
    QQuaternion quat_t;
};

class SimulationThread : public QThread
{
    Q_OBJECT

public:
    SimulationThread(const SimulationSettings& s);

    virtual ~SimulationThread()
    {
        exit();
        wait();
    }

    void update() noexcept;
    float get_position() noexcept;
    float get_init_position() noexcept;
    unsigned long long update_count = 0;

    void updateSettings(const SimulationSettings& s);

public slots:
    void restart(const SimulationSettings& s);
signals:
    void positionChanged(QQuaternion rotation);

private:
    void precalculate();
    RK4_ret rk4(const QVector3D&);

    QTimer s_timer;
    SimulationSettings settings;
    QVector3D initial_center;
    QQuaternion current_rotation;
    QVector3D current_av;

    // precalculated elements
    QMatrix3x3 tensor;
    QMatrix3x3 tensor_inverted;
    float mass;

protected:
    void run() override;
};

#endif // SPRINGTHREAD_H
