#ifndef SPRINGTHREAD_H
#define SPRINGTHREAD_H

#include <QThread>
#include <QTimer>
#include <QVector3D>
#include <array>
#include <random>

template <int N, typename T>
using triple_array = std::array<std::array<std::array<T, N>, N>, N>;

using points_positions_t = triple_array<4, QVector3D>;
using velocities_t = triple_array<4, QVector3D>;

struct euler_out_t
{
    QVector3D vel_t;
    QVector3D pos_t;
};

struct SimulationSettings
{
    // interface musi dawać możliwość dodania:
    //  1. wyświetlanie punktów kontrolnych
    bool show_control_points { true };
    //  2. wyświetlanie ramki sterującej i połączeń z kostką
    bool show_control_frame { true };
    //  3. prostopadłościanu ograniczającego
    bool show_constraint { true };
    //  4. cieniowanej kostki beziera
    bool show_jelly { true };
    //  5. cieniowanej zdeformowanej bryły
    bool show_inner { true };
    //  6. zaburzenie
    bool use_randomization { true };
    //  7. zmiana masy punktów kontrolnych
    float mass { 0.1f };
    //  8. wartość tłumienia k
    float k { 0.4f };
    //  9. współczynnik sprężystości c1 (połączeń między masami)
    float c1 { 30.0f };
    //  10. c2 (kostka a ramka sterująca)
    float c2 { 50.0f };
    //  11. początkowe zaburzenie, określające maksymalną wartość losowanych prędkości lub odchyleń
    float random_max { 0.3f };

    // 12. dt in ms
    int dt_ms { 1 };

    // 13. l0
    float l0 { 1.0f };
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
    void changeFramePosition(QVector3D f);
signals:
    void pointsPositionChanged(points_positions_t pos);

private:
    void precalculate();
    void initialize_data();

    QTimer s_timer;
    SimulationSettings settings;
    points_positions_t current_positions;
    velocities_t current_velocities;
    float dt;
    float pre1_c1;
    float pre1_c2;
    float pre2;
    float mul;
    QVector3D frame_pos;
    std::random_device device;
    std::mt19937 generator { device() };

protected:
    void run() override;
};

#endif // SPRINGTHREAD_H
