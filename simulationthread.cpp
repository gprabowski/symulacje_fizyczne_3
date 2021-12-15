#include "simulationthread.h"
#include <QDebug>
#include <QtMath>
#include <cmath>
#include <type_traits>

using perm = std::array<int, 3>;
constexpr std::array<perm, 18> permutations {
    perm { -1, 0, -1 },
    perm { 0, 0, -1 },
    perm { 1, 0, -1 },
    perm { -1, 0, 0 },
    perm { 1, 0, 0 },
    perm { -1, 0, 1 },
    perm { 0, 0, 1 },
    perm { 1, 0, 1 },
    perm { 0, -1, -1 },
    perm { 0, 1, -1 },
    perm { 0, -1, 0 },
    perm { 0, 1, 0 },
    perm { 0, -1, 1 },
    perm { 0, 1, 1 },
    perm { -1, -1, 0 },
    perm { 1, -1, 0 },
    perm { -1, 1, 0 },
    perm { 1, 1, 0 },
};

void SimulationThread::initialize_data()
{
    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            for (int k = 0; k < 4; ++k)
            {
                // odejmuje 1.5 żeby pozycje startowe były takie żeby nasz żelek
                // miał swoje centrum w (0, 0, 0)
                current_positions[i][j][k] = QVector3D((i - 1.5f) * settings.l0, (j - 1.5f) * settings.l0, (k - 1.5f) * settings.l0);
                current_velocities[i][j][k] = 0.0f;
                // implement random initialization
                throw std::logic_error("not implemented random initialization");
            }
        }
    }
}

void SimulationThread::precalculate()
{
    // Precalculate I with respect to the fixed vertex
    mul = dt / settings.mass;
    pre1_c1 = settings.c1 * mul;
    pre1_c2 = settings.c2 * mul;
    pre2 = settings.k * mul;
}

SimulationThread::SimulationThread(const SimulationSettings& s)
{
    settings = s;
    s_timer.setTimerType(Qt::PreciseTimer);
    connect(&s_timer, &QTimer::timeout, this, &SimulationThread::update);
    s_timer.moveToThread(this);
}

void SimulationThread::update() noexcept
{
    point_positions_t ret;
    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            for (int k = 0; k < 4; ++k)
            {
                QVector3D pos_t;
                float vel_t { 0.0f };
                const auto& curr_pos = current_positions[i][j][k];
                const auto& curr_vel = current_velocities[i][j][k];
                // accumulation for pos[i, j, k]

                // pre computed constant terms
                // in the xz plane (8 springs)
                // in the yz plane without xz elements (6 springs)
                // in the xy plane without xz and yz elements (4 springs)
                for (const auto p : permutations)
                {
                    try
                    {
                        const auto& pos2 = current_positions.at(i + p[0]).at(j + p[1]).at(k + p[2]);
                        const auto dist = (curr_pos - pos2).length();
                        pos_t += curr_vel * dt * (pos2 - curr_pos).normalized();
                        // velocity += (elasticity * (l0 - position) - stickiness * velocity + h) * 1/weight * dt;
                        const auto acceleration = settings.c1 * (settings.l0 - dist) - settings.k * curr_vel;
                        vel_t += acceleration;
                    }
                    catch (std::out_of_range& e)
                    {
                        continue;
                    }
                }

                // musimy do innej struktury zapisywać
                // żeby cały krok wykonywał się na tych samych danych
                // (każde połączenie wchodzi w 2 interakcje, nie możemy przesunąć jednej strony
                // i potem policzyć drugiej na nowych danych)
                ret[i][j][k] = curr_pos + pos_t;
                current_velocities[i][j][k] += vel_t;
            }
        }
    }
    // chyba nie unikniemy kopii
    emit positionChanged(ret);
    current_positions = std::move(ret);
    throw std::logic_error("not implemented");
}

void SimulationThread::updateSettings(const SimulationSettings& s)
{
    settings = s;
    dt = s.dt_ms / 1000.f;
    precalculate();
}

void SimulationThread::restart(const SimulationSettings& s)
{
    exit();
    wait();
    settings = s;
    dt = s.dt_ms / 1000;
    precalculate();
    initialize_data();
    throw std::logic_error("not implemented");
    start();
}

void SimulationThread::run()
{
    s_timer.start(settings.dt_ms);
    exec();
}

void SimulationThread::frame_changed(frame_position_t frame)
{
    throw std::logic_error("not implemented");
}
