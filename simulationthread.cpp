#include "simulationthread.h"
#include <QDebug>
#include <QtMath>
#include <cmath>
#include <iostream>
#include <type_traits>
#include <cmath>

using perm = std::array<int, 4>;
const float sq2 = std::sqrt(2);

struct elem {
    perm coord;
    float mult;
};

static const std::array<elem, 18> permutations {
    elem { { -1, 0, -1}, sq2},
    elem { { 0, 0, -1 }, 1},
    elem { { 1, 0, -1 }, sq2},
    elem { { -1, 0, 0 }, 1},
    elem { { 1, 0, 0 }, 1},
    elem { { -1, 0, 1 }, sq2},
    elem { { 0, 0, 1 }, 1},
    elem { { 1, 0, 1 }, sq2},
    elem { { 0, -1, -1 }, sq2},
    elem { { 0, 1, -1 }, sq2},
    elem { { 0, -1, 0 }, 1},
    elem { { 0, 1, 0 }, 1},
    elem { { 0, -1, 1 }, sq2},
    elem { { 0, 1, 1 }, sq2},
    elem { { -1, -1, 0 }, sq2},
    elem { { 1, -1, 0 }, sq2},
    elem { { -1, 1, 0 }, sq2},
    elem { { 1, 1, 0 }, sq2}
};


inline constexpr bool is_corner(int a, int b, int c) {
   return ((a == 0 || a == 3) && (b == 0 || b == 3) && (c == 0 || c == 3));
}

void SimulationThread::initialize_data()
{
    std::uniform_real_distribution<float> distribution(-settings.random_max, settings.random_max);
    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            for (int k = 0; k < 4; ++k)
            {
                // odejmuje 1.5 żeby pozycje startowe były takie żeby nasz żelek
                // miał swoje centrum w (0, 0, 0)
                current_positions[i][j][k] = QVector3D((i - 1.5f) * settings.l0, (j - 1.5f) * settings.l0, (k - 1.5f) * settings.l0);
                current_velocities[i][j][k] = QVector3D(0.0f, 0.0f, 0.0f);
                // implement random initialization
                if (settings.use_randomization)
                {
                    current_positions[i][j][k] += QVector3D(distribution(generator), distribution(generator), distribution(generator));
                    current_velocities[i][j][k] += QVector3D(distribution(generator), distribution(generator), distribution(generator));
                }
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
    points_positions_t ret;
    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            for (int k = 0; k < 4; ++k)
            {
                const auto& curr_pos = current_positions[i][j][k];
                auto& curr_vel = current_velocities[i][j][k];

                // musimy do innej struktury zapisywać
                // żeby cały krok wykonywał się na tych samych danych
                // (każde połączenie wchodzi w 2 interakcje, nie możemy przesunąć jednej strony
                // i potem policzyć drugiej na nowych danych)

                ret[i][j][k] = curr_pos + dt * current_velocities[i][j][k];
                auto& new_pos = ret[i][j][k];


                auto one_time_term = -settings.k * curr_vel;
                QVector3D iterated_term;

                // accumulation for pos[i, j, k]
                // pre computed constant terms
                // in the xz plane (8 springs)
                // in the yz plane without xz elements (6 springs)
                // in the xy plane without xz and yz elements (4 springs)
                for (const auto p : permutations)
                {
                    auto idx1 = i + p.coord[0];
                    auto idx2 = j + p.coord[1];
                    auto idx3 = k + p.coord[2];
                    if (idx1 >= 0 && idx1 < 4 && idx2 >= 0 && idx2 < 4 && idx3 >= 0 && idx3 < 4)
                    {
                        const auto& pos2 = current_positions[idx1][idx2][idx3];
                        const auto dist = (pos2 - new_pos).length();
                        // velocity += (elasticity * (l0 - position) - stickiness * velocity + h) * 1/weight * dt;
                        iterated_term += (-settings.c1 * (settings.l0 * p.mult - dist)) * (pos2 - new_pos).normalized();
                    }
                }

                if(is_corner(i, j, k)) {
                    // add velocity change caused by frame
                    auto frame_vec = ((frame_pos + QVector3D(i -1.5, j - 1.5, k - 1.5)) - new_pos);
                    float frame_dist = frame_vec.length();
                    iterated_term += (settings.c2 * (frame_dist)) * frame_vec.normalized();
                }

                curr_vel += (one_time_term + iterated_term) * 1/settings.mass * dt;

            }
        }
    }
    // chyba nie unikniemy kopii
    emit pointsPositionChanged(ret);
    current_positions = std::move(ret);
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
    dt = s.dt_ms / 1000.f;
    precalculate();
    initialize_data();
    // throw std::logic_error("not implemented");
    start();
}

void SimulationThread::run()
{
    s_timer.start(settings.dt_ms);
    exec();
}

void SimulationThread::changeFramePosition(QVector3D pos)
{
    frame_pos = pos;
}
